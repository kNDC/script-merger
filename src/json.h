#pragma once

// IO, strings
#include <cwctype>
#include <cstring>
#include <iostream>
#include <string>

// Collections
#include <vector>
#include <map>

// Utilities
#include <variant>
#include <cassert>

namespace json
{
    template <typename T>
    class Node;

    template <typename T>
    class Document;

    // Объявления Dict и Array
    template <typename T>
    using Array = std::vector<Node<T>>;

    // Case-neutral string comparator
    template <typename T>
    struct CN_Less
    {
        bool operator()(std::basic_string_view<T> sv1,
            std::basic_string_view<T> sv2) const
        {
            size_t min_size = (sv1.size() < sv2.size()) ? sv1.size() : sv2.size();

            for (size_t i = 0; i < min_size; ++i)
            {
                T c1 = std::towlower(sv1[i]);
                T c2 = std::towlower(sv2[i]);
                if (c1 == c2) continue;
                if (c1 < c2) return true;
                return false;
            }

            return sv1.size() < sv2.size();
        }
    };

    template <typename T>
    using Dict = std::map<std::basic_string<T>, Node<T>,
        CN_Less<T>>;

    // Ошибка разбора json
    class parsing_error : public std::runtime_error
    {
    public:
        using runtime_error::runtime_error;
    };

    const static char* indent = "\t";

    template <typename T>
    using Value = std::variant<std::nullptr_t,
        bool, int, double,
        std::basic_string<T>,
        Array<T>, Dict<T>>;

    template <typename T>
    class Node final : Value<T>
    {
    public:
        // Открытие доступа к конструкторам variant
        using Value<T>::Value;

        // Открытие доступа к swap
        using Value<T>::swap;

    private:
        static void EscapeChars(std::basic_ostream<T>& os, std::basic_string_view<T> sv)
        {
            for (char c : sv)
            {
                switch (c)
                {
                case '\r':
                    os << "\\r";
                    break;
                case '\n':
                    os << "\\n";
                    break;
                case '\t':
                    os << "\\t";
                    break;
                case '\"':
                    os << "\\\"";
                    break;
                case '\\':
                    os << "\\\\";
                    break;
                default:
                    os << c;
                    break;
                }
            }
        }

        struct PrintContext
        {
            std::basic_ostream<T>& os;
            mutable size_t indent_count = 0;

            PrintContext(std::basic_ostream<T>& os, size_t indent_count = 0) :
                os(os),
                indent_count(indent_count)
            {
            }
        };

        struct ValuePrinter
        {
            const PrintContext& pc;

            void AddIndentation(std::basic_string_view<T> prefix = {},
                std::basic_string_view<T> suffix = {}) const
            {
                pc.os << prefix;
                size_t i = pc.indent_count + 1;
                while (--i) pc.os << indent;
                pc.os << suffix;
            }

            void operator()(std::nullptr_t) const
            {
                pc.os << "null";
            }

            void operator()(bool value) const
            {
                pc.os << ((value) ? "true" : "false");
            }

            // Печать чисел
            template <typename Value>
            void operator()(Value value) const
            {
                pc.os << value;
            }

            // Печать строк
            void operator()(const std::basic_string<T>& string) const
            {
                pc.os << '\"';
                EscapeChars(pc.os, string);
                pc.os << '\"';
            }

            // Печать массивов
            void operator()(const Array<T>& array) const;

            // Печать словарей
            void operator()(const Dict<T>& map) const;
        };

        std::basic_ostream<T>& Print(ValuePrinter) const;

    public:
        // Constructors are inherited from std::variant

        // Node is...
        bool IsNull() const;

        bool IsBool() const;
        bool IsInt() const;
        bool IsPureDouble() const;
        bool IsDouble() const;

        bool IsString() const;

        bool IsArray() const;
        bool IsMap() const;

        // Node as...
        bool AsBool() const;
        int AsInt() const;
        double AsDouble() const;
        std::basic_string<T>& AsString();
        const std::basic_string<T>& AsString() const;

        Array<T>& AsArray();
        const Array<T>& AsArray() const;

        Dict<T>& AsMap();
        const Dict<T>& AsMap() const;

        operator bool() const { return AsBool(); }
        operator int() const { return AsInt(); }
        operator double() const { return AsDouble(); }

        operator std::basic_string<T>& () { return AsString(); }
        operator const std::basic_string<T>& () const { return AsString(); }

        operator Array<T>& () { return AsArray(); }
        operator const Array<T>& () const { return AsArray(); }

        operator Dict<T>& () { return AsMap(); }
        operator const Dict<T>& () const { AsMap(); }

        void Swap(Node& other);

        bool operator==(const Node& other) const;
        bool operator!=(const Node& other) const;
        std::basic_ostream<T>& operator<<(std::basic_ostream<T>&) const;
    };

    template <typename T>
    class Document
    {
    private:
        Node<T> root_;

    public:
        explicit Document(Node<T> root);

        Node<T>& GetRoot();
        const Node<T>& GetRoot() const;

        Document& operator=(const Document& other);
        Document& operator=(Document&& other);

        void Print(std::basic_ostream<T>& os) const;
        void Swap(Document& other);

        bool operator==(const Document& other) const;
        bool operator!=(const Document& other) const;
    };

    template <typename T>
    std::basic_string<T> Convert(const char* chars)
    {
        return { chars, chars + strlen(chars) };
    }

    template <typename T>
    bool CheckIfNoSuffix(typename std::char_traits<T>::char_type c)
    {
        using CharType = typename std::char_traits<T>::char_type;

        return (c == (CharType)EOF) ||
            (c == '\r') || (c == '\n') ||
            (c == '\t') || (c == ' ') ||
            (c == ']') || (c == '}') ||
            (c == ':') || (c == ',');
    }

    template <typename T>
    void SkipSpecChars(std::basic_istream<T>& is)
    {
        using CharType = typename std::char_traits<T>::char_type;

        CharType c;
        while (c = is.peek(),
            (c == '\r') || (c == '\n') ||
            (c == '\t') || (c == ' ')) is.get();
    }

    template <typename T>
    Document<T> Load(std::basic_istream<T>&);

    // Печать массивов
    template <typename T>
    void Node<T>::ValuePrinter::operator()(const Array<T>& array) const
    {
        pc.os << "[\n";
        ++pc.indent_count;

        std::basic_string<T> sep;

        for (const Node& node : array)
        {
            AddIndentation(sep); sep = Convert<T>(",\r\n");
            node.Print({ {pc.os, pc.indent_count} });
        }

        --pc.indent_count;
        AddIndentation(Convert<T>("\r\n"), Convert<T>("]"));
    }

    // Печать словарей
    template <typename T>
    void Node<T>::ValuePrinter::operator()(const Dict<T>& map) const
    {
        pc.os << "{\n";
        ++pc.indent_count;

        std::basic_string<T> sep;

        for (const std::pair<const std::basic_string<T>, Node>& pair : map)
        {
            AddIndentation(sep); sep = Convert<T>(",\r\n");
            operator()(pair.first);
            pc.os << ": "; pair.second.Print({ {pc.os, pc.indent_count} });
        }

        --pc.indent_count;
        AddIndentation(Convert<T>("\r\n"), Convert<T>("}"));
    }

    template <typename T>
    bool Node<T>::IsNull() const
    {
        return std::holds_alternative<std::nullptr_t>(*this);
    }

    template <typename T>
    bool Node<T>::IsBool() const
    {
        return std::holds_alternative<bool>(*this);
    }

    template <typename T>
    bool Node<T>::IsInt() const
    {
        return std::holds_alternative<int>(*this);
    }

    template <typename T>
    bool Node<T>::IsPureDouble() const
    {
        return std::holds_alternative<double>(*this);
    }

    template <typename T>
    bool Node<T>::IsDouble() const
    {
        return IsPureDouble() || IsInt();
    }

    template <typename T>
    bool Node<T>::IsString() const
    {
        return std::holds_alternative<std::basic_string<T>>(*this);
    }

    template <typename T>
    bool Node<T>::IsArray() const
    {
        return std::holds_alternative<Array>(*this);
    }

    template <typename T>
    bool Node<T>::IsMap() const
    {
        return std::holds_alternative<Dict<T>>(*this);
    }

    template <typename T>
    bool Node<T>::AsBool() const
    {
        if (!IsBool()) throw std::logic_error("Incompatible node type!");
        return std::get<bool>(*this);
    }

    template <typename T>
    int Node<T>::AsInt() const
    {
        if (!IsInt()) throw std::logic_error("Incompatible node type!");
        return std::get<int>(*this);
    }

    template <typename T>
    double Node<T>::AsDouble() const
    {
        if (IsPureDouble()) return std::get<double>(*this);
        if (IsInt()) return (double)std::get<int>(*this);

        throw std::logic_error("Incompatible node type!");
    }

    template <typename T>
    std::basic_string<T>& Node<T>::AsString()
    {
        if (!IsString()) throw std::logic_error("Incompatible node type!");
        return std::get<std::basic_string<T>>(*this);
    }

    template <typename T>
    const std::basic_string<T>& Node<T>::AsString() const
    {
        if (!IsString()) throw std::logic_error("Incompatible node type!");
        return std::get<std::basic_string<T>>(*this);
    }

    template <typename T>
    Array<T>& Node<T>::AsArray()
    {
        if (!IsArray()) throw std::logic_error("Incompatible node type!");
        return std::get<Array<T>>(*this);
    }

    template <typename T>
    const Array<T>& Node<T>::AsArray() const
    {
        if (!IsArray()) throw std::logic_error("Incompatible node type!");
        return std::get<Array<T>>(*this);
    }

    template <typename T>
    Dict<T>& Node<T>::AsMap()
    {
        if (!IsMap()) throw std::logic_error("Incompatible node type!");
        return std::get<Dict<T>>(*this);
    }

    template <typename T>
    const Dict<T>& Node<T>::AsMap() const
    {
        if (!IsMap()) throw std::logic_error("Incompatible node type!");
        return std::get<Dict<T>>(*this);
    }

    template<typename T>
    inline void Node<T>::Swap(Node& other)
    {
        Value<T>::swap(other);
    }

    template <typename T>
    bool Node<T>::operator==(const Node& other) const
    {
        if (this == &other) return true;
        if (this->index() != other.index()) return false;
        return (const Value<T>&) * this == (const Value<T>&)other;
    }

    template <typename T>
    bool Node<T>::operator !=(const Node& other) const
    {
        return !(*this == other);
    }

    template <typename T>
    std::basic_ostream<T>& Node<T>::Print(ValuePrinter printer) const
    {
        std::visit(printer, (Value<T>&) * this);
        return printer.pc.os;
    }

    template <typename T>
    std::basic_ostream<T>& Node<T>::operator<<(std::basic_ostream<T>& os) const
    {
        Print(ValuePrinter{ {os, 0} });
        return os;
    }

    template <typename T>
    std::basic_ostream<T>& operator<<(std::basic_ostream<T>& os, const Node<T>& node)
    {
        return node.operator<<(os);
    }

    template <typename T>
    Node<T> LoadNode(std::basic_istream<T>& is);

    template <typename T>
    Node<T> LoadNull(std::basic_istream<T>& is)
    {
        typename std::char_traits<T>::char_type buffer[3];
        buffer[0] = is.get();
        buffer[1] = is.get();
        buffer[2] = is.get();

        if (buffer[0] != 'u' || buffer[1] != 'l' || buffer[2] != 'l')
        {
            throw parsing_error("Unrecognised token (typo in null?)");
        }

        // Отсекаем приколы вроде nullnull
        if (!CheckIfNoSuffix<T>(is.peek()))
        {
            throw parsing_error("Unrecognised token (typo in null?)");
        }

        return Node<T>();
    }

    template <typename T>
    Node<T> LoadBool(std::basic_istream<T>& is)
    {
        using CharType = typename std::char_traits<T>::char_type;

        CharType buffer[3];
        buffer[0] = is.get();
        buffer[1] = is.get();
        buffer[2] = is.get();

        if (!((buffer[0] != 'r') ||
            (buffer[1] != 'u') ||
            (buffer[2] != 'e')))
        {
            // Отсекаем приколы вроде truetrue
            if (CheckIfNoSuffix<T>(is.peek()))
            {
                return Node<T>(true);
            }
        }

        if (!((buffer[0] != 'a') ||
            (buffer[1] != 'l') ||
            (buffer[2] != 's')))
        {
            // Отсекаем приколы вроде falsefalse
            if (is.get() == 'e' &&
                CheckIfNoSuffix<T>(is.peek()))
            {
                return Node<T>(false);
            }
        }

        throw parsing_error("Unrecognised token (typo in true/false?)");
    }

    template <typename T>
    Node<T> LoadNumber(std::basic_istream<T>& is)
    {
        std::basic_string<T> int_part;
        std::basic_string<T> frac_part;
        std::basic_string<T> exp_part;

        // Целая часть
        typename std::char_traits<T>::int_type c = is.get();
        if (c == '-')
        {
            int_part += c;

            c = is.get();
            if (!std::isdigit(c))
            {
                throw parsing_error("A number cannot consist only of the minus sign");
            }
        }

        if (c == '0')
        {
            int_part += c;

            if (std::isdigit(is.peek()))
            {
                throw parsing_error("Numbers with leading zeros are not allowed");
            }
        }
        else
        {
            int_part += c;

            while (std::isdigit(is.peek()))
            {
                int_part += is.get();
            }
        }

        // Дробная часть
        if (c = is.peek(); c == '.')
        {
            // Пропускаем точку
            is.get();

            while (std::isdigit(is.peek()))
            {
                frac_part += is.get();
            }
        }

        // Степень после мантиссы
        if (c = is.peek(); (c == 'e') || (c == 'E'))
        {
            // Пропускаем e/E
            is.get();

            if (c = is.peek(); (c == '+') || (c == '-'))
            {
                exp_part += is.get();
            }

            while (std::isdigit(is.peek()))
            {
                exp_part += is.get();
            }

            if (!CheckIfNoSuffix<T>(is.peek()))
            {
                throw parsing_error("Unexpected end of a number token");
            }
        }
        else if (!CheckIfNoSuffix<T>(c))
        {
            throw parsing_error("Unexpected end of a number token");
        }

        if (frac_part.empty() && exp_part.empty())
        {
            try
            {
                return { std::stoi(int_part) };
            }
            catch (...)
            {
                throw parsing_error("Failed number convertion");
            }
        }
        else
        {
            std::basic_string<T> text_to_number = int_part;

            if (!frac_part.empty())
            {
                text_to_number += '.';
                text_to_number += frac_part;
            }

            if (!exp_part.empty())
            {
                text_to_number += 'e';
                text_to_number += exp_part;
            }

            try
            {
                return { std::stod(text_to_number) };
            }
            catch (...)
            {
                throw parsing_error("Failed number conversion");
            }
        }
    }

    // Считывает содержимое строки JSON-документа
    // Функцию следует использовать после считывания открывающего символа ":
    template <typename T>
    Node<T> LoadString(std::basic_istream<T>& is)
    {
        using CharType = typename std::char_traits<T>::char_type;
        std::basic_string<T> string;

        while (true)
        {
            const CharType c = is.get();

            if (c == (CharType)EOF)
            {
                // Поток закончился до того, как встретили закрывающую кавычку
                throw parsing_error("Unexpected end of a line (runaway closing \"?)");
            }

            if (c == '"')
            {
                // Встретили закрывающую кавычку
                break;
            }
            else if (c == '\\')
            {
                // Встретили начало escape-последовательности
                const CharType escaped_char = is.get();
                if (escaped_char == (CharType)EOF)
                {
                    // Поток завершился сразу после символа обратной косой черты
                    throw parsing_error("String parsing error");
                }

                // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                switch (escaped_char)
                {
                case 'n':
                    string.push_back('\n');
                    break;
                case 't':
                    string.push_back('\t');
                    break;
                case 'r':
                    string.push_back('\r');
                    break;
                case '"':
                    string.push_back('"');
                    break;
                case '\\':
                    string.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw parsing_error("Unrecognised escape sequence \\" + escaped_char);
                }
            }
            else if (c == '\n' || c == '\r')
            {
                // Строковый литерал внутри JSON не может прерываться символами \r или \n
                throw parsing_error("Unexpected end of a line");
            }
            else
            {
                // Просто считываем очередной символ и помещаем его в получаемую строку
                string.push_back(c);
            }
        }

        return Node<T>(string);
    }

    template <typename T>
    Node<T> LoadArray(std::basic_istream<T>& is)
    {
        using CharType = typename std::char_traits<T>::char_type;

        Array<T> array;
        CharType c = is.peek();

        for (; !(c == ']' || c == (CharType)EOF);
            c = is.peek())
        {
            // Пропускаем запятую
            if (c == ',') is.get();

            array.emplace_back(LoadNode(is));
            SkipSpecChars<T>(is);
        }

        if (c == (CharType)EOF)
        {
            throw parsing_error("Unexpected end of an array (did you forget the closing bracket ']'?)");
        }

        // Пропускаем закрывающую скобку
        is.get();

        return Node<T>(std::move(array));
    }

    template <typename T>
    Node<T> LoadDict(std::basic_istream<T>& is)
    {
        using CharType = typename std::char_traits<T>::char_type;

        Dict<T> map;
        CharType c = is.peek();

        for (; !(c == '}' || c == (CharType)EOF); c = is.peek())
        {
            if (c == '\"')
            {
                // Пропускаем открывающие кавычки, считываем ключ
                c = is.get();
                std::basic_string<T> key = LoadString(is).AsString();
                SkipSpecChars<T>(is);

                // Достигаем разделителя ':', проверяем
                c = is.get();
                if (c != ':')
                {
                    throw std::logic_error("Error parsing a map (':' was expected)");
                }
                SkipSpecChars<T>(is);

                map.emplace(std::make_pair(std::move(key), LoadNode(is)));
                SkipSpecChars<T>(is);

                // Достигаем разделителя ',', проверяем
                c = is.peek();
                if (c == ',') c = is.get();
            }

            SkipSpecChars<T>(is);
        }

        if (c == (CharType)EOF)
        {
            throw parsing_error("Unexpected end of a map (did you forget the closing brace '}'?)");
        }

        // Пропускаем закрывающую скобку
        is.get();

        return Node<T>(std::move(map));
    }

    template <typename T>
    Node<T> LoadNode(std::basic_istream<T>& is)
    {
        using CharType = typename std::char_traits<T>::char_type;

        SkipSpecChars<T>(is);

        CharType c = is.get();
        switch (c)
        {
        case '[':
            return LoadArray(is);
        case '{':
            return LoadDict(is);
        case '"':
            return LoadString(is);
        case 'n':
            return LoadNull(is);
        case 't':
        case 'f':
            return LoadBool(is);
        default:
            is.putback(c);
            return LoadNumber(is);
        }
    }

    template <typename T>
    Document<T>::Document(Node<T> root) :
        root_(std::move(root))
    {
    }

    template <typename T>
    Node<T>& Document<T>::GetRoot()
    {
        return root_;
    }

    template <typename T>
    const Node<T>& Document<T>::GetRoot() const
    {
        return root_;
    }

    template <typename T>
    Document<T>& Document<T>::operator=(const Document& other)
    {
        root_ = other.root_;
        return *this;
    }

    template <typename T>
    Document<T>& Document<T>::operator=(Document&& other)
    {
        Swap(other);
        return *this;
    }

    template <typename T>
    void Document<T>::Swap(Document& other)
    {
        root_.Swap(other.root_);
    }

    template <typename T>
    bool Document<T>::operator==(const Document& other) const
    {
        return root_ == other.root_;
    }

    template <typename T>
    bool Document<T>::operator!=(const Document& other) const
    {
        return !(*this == other);
    }

    template <typename T>
    Document<T> Load(std::basic_istream<T>& is)
    {
        return Document{ LoadNode(is) };
    }

    template <typename T>
    void Document<T>::Print(std::basic_ostream<T>& os) const
    {
        os << GetRoot();
    }
} // namespace json