# Dashboard is opened for submissions for a 24 hour period starting at
# the specified NIGHLY_START_TIME. Time is specified in 24 hour format.
SET (NIGHTLY_START_TIME "21:00:00 EDT")

# Dart server to submit results (used by client)
SET (DROP_METHOD "http")
SET (DROP_SITE "public.kitware.com")
SET (DROP_LOCATION "/cgi-bin/HTTPUploadDartFile.cgi")
SET (TRIGGER_SITE "http://${DROP_SITE}/cgi-bin/Submit-Fltk-TestingResults.pl")

# Project Home Page
SET (PROJECT_URL "https://www.fltk.org")

# Dart server configuration
SET (ROLLUP_URL "http://${DROP_SITE}/cgi-bin/fltk-rollup-dashboard.sh")
SET (CVS_WEB_URL "http://cvs.sourceforge.net/viewcvs.py/fltk/fltk/")
SET (CVS_WEB_CVSROOT "fltk")

SET (USE_GNATS "On")
SET (GNATS_WEB_URL "https://www.fltk.org/bugs.php")

# Continuous email delivery variables
SET (CONTINUOUS_FROM "fltk-dashboard@public.kitware.com")
SET (SMTP_MAILHOST "public.kitware.com")
SET (CONTINUOUS_MONITOR_LIST "fltk-dashboard@public.kitware.com")
SET (CONTINUOUS_BASE_URL "http://public.kitware.com/Fltk/Testing")

SET (DELIVER_BROKEN_BUILD_EMAIL_WITH_TEST_FAILURES ON)
SET (DELIVER_BROKEN_BUILD_EMAIL "Continuous Nightly")
SET (EMAIL_FROM "fltk-dashboard@public.kitware.com")
SET (DARTBOARD_BASE_URL "http://public.kitware.com/Fltk/Testing")

SET (DELIVER_BROKEN_BUILD_EMAIL_WITH_CONFIGURE_FAILURES 1)
SET (DELIVER_BROKEN_BUILD_EMAIL_WITH_BUILD_ERRORS 1)
SET (DELIVER_BROKEN_BUILD_EMAIL_WITH_BUILD_WARNINGS 1)
SET (DELIVER_BROKEN_BUILD_EMAIL_WITH_TEST_NOT_RUNS 1)
SET (DELIVER_BROKEN_BUILD_EMAIL_WITH_TEST_FAILURES 1)

