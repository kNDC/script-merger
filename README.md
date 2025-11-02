# script-merger
## Description and requirements
A console application for merging marked and annotated class test/essay scripts with front pages containing score distribution. Saved me oh so many hours in my teaching.

The application requires 3 compulsory directories
1. Directory with annotated scripts;
2. Directory with front pages;
3. Directory where merged files are saved (can be created automatically if missing, as no files are read from it);

All of the above have to be entered in settings, which the user can be guided through at the start of the program.

## Brief overview of additional features

The university where I used to work uses two different Id registers: university email based ( name initials + counter value which guarantees uniqueness, e.g. zzz999@uni-email.ac.uk) and matriculation Id based (the matricualtion Id is a unique number assigned to a student at the start of their studies).  Scripts tend to come bearing email based credentials, typically in the middle of identifying text (e.g., zzz999-Technical_Assignment.pdf, zzz999-Essay.pdf) while batch uploading them requires using matriculation Ids, and front pages are identified with matriculation Ids too.  In order to account for these facts of life, the application has the following features

* Expecting patterns in the naming of scripts.  To indicate this, the pattern has to be entered in settings using the asterisk as the stand-in for the email (e.g., *-Technical_Assignment);
* Reading a map file from emails to matriculation Ids. Its lines are expected to have the email first and matriculation Id second, separated with a tab (e.g., zzz999  123456789).  To indicate this, the name of the map file has to be entered in the application's settings;

The application requires the [PoDoFo]([url](https://github.com/podofo/podofo)) library to operate.
