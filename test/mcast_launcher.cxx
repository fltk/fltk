/* This is just a simple wrapper app that launches two independent processes then exits...
 * I use it because the fltk test/demo program launches one process per button, and I wanted
 * to launch several in my multicast/Fl::add_fd() test harness.
 *
 * This code also shows how to find the "home" directory for any executable in a cross-platform
 * way, which can be handy for finding resources or help files.
 */
#ifdef WIN32
#include <windows.h> /* GetModuleFileName */
#endif /* WIN32 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef __APPLE__ /* assume this is OSX */
#include <sys/param.h>
#include <mach-o/dyld.h> /* _NSGetExecutablePath : must add -framework CoreFoundation to link line */
#include <string.h>
# ifndef PATH_MAX
#  define PATH_MAX MAXPATHLEN
# endif
#endif /* APPLE */

#ifndef PATH_MAX
#  define PATH_MAX 2048
#endif
/*******************************************************************************************/
static int get_app_path (char *pname, size_t pathsize)
{
    long result;

#if defined (WIN32)
    result = GetModuleFileName(NULL, pname, pathsize);
    if (result > 0) {
        /* fix up the dir slashes... */
        int len = strlen(pname);
        int idx;
        for (idx = 0; idx < len; idx++) {
            if (pname[idx] == '\\') pname[idx] = '/';
        }
        if ((access(pname, 0) == 0)) {
            return 0; /* file exists, return OK */
        }
        /*else name doesn't seem to exist, return FAIL (falls through) */
    }

#elif defined (SOLARIS) // we used to set this for our all our Sun builds - I wonder what Sun set...?
	char *p = getexecname();
	if (p) {
		/* According to the Sun manpages, getexecname will "normally" return an */
		/* absolute path - BUT might not... AND that IF it is not, pre-pending */
		/* getcwd() will "usually" be the correct thing... Urgh! */

		/* check pathname is absolute (begins with a / ) */
		if (p[0] == '/') { /* assume this means we have an absolute path */
			strncpy(pname, p, pathsize);
			if ((access(pname, 0) == 0))
				return 0; /* file exists, return OK */
		} else { /* if not, prepend getcwd() then check if file exists */
			getcwd(pname, pathsize);
			result = strlen(pname);
			strncat(pname, "/", (pathsize - result));
			result ++;
			strncat(pname, p, (pathsize - result));

			if ((access(pname, 0) == 0))
				return 0; /* file exists, return OK */
			/*else name doesn't seem to exist, return FAIL (falls through) */
		}
	}

#elif defined (__APPLE__) /* assume this is OSX */
/* extern int _NSGetExecutablePath(char *buf, unsigned long *bufsize);

    _NSGetExecutablePath  copies  the  path  of the executable
    into the buffer and returns 0 if the path was successfully
    copied  in the provided buffer. If the buffer is not large
    enough, -1 is returned and the  expected  buffer  size  is
    copied  in  *bufsize.  Note that _NSGetExecutablePath will
    return "a path" to the executable not a "real path" to the
    executable.  That  is  the path may be a symbolic link and
    not the real file. And with  deep  directories  the  total
    bufsize needed could be more than MAXPATHLEN.
*/
    int status = -1;
    char *given_path = (char *)malloc(MAXPATHLEN * 2);
    if (!given_path) return status;

    pathsize = MAXPATHLEN * 2;
    result = _NSGetExecutablePath(given_path, (uint32_t *)&pathsize);
    if (result == 0) { /* OK, we got something - now try and resolve the real path... */
        if (realpath(given_path, pname) != NULL) {
            if ((access(pname, 0) == 0)) {
                status = 0; /* file exists, return OK */
            }
        }
    }
    free (given_path);
    return status;

#else // just assume this is linux for now - not valid - what about BSD's etc...?
    /* Oddly, the readlink(2) man page says no NULL is appended. */
    /* So you have to do it yourself, based on the return value: */
    pathsize --; /* Preserve a space to add the trailing NULL */
    result = readlink("/proc/self/exe", pname, pathsize);

    if (result > 0) {
        pname[result] = 0; /* add a terminating NULL */

        if ((access(pname, 0) == 0)) {
            return 0; /* file exists, return OK */
        }
        /*else name doesn't seem to exist, return FAIL (falls through) */
    }
#endif /* LINUX (assumed!) */

    return -1; /* Path Lookup Failed */
} // get_app_path

/*******************************************************************************************/
static void app_launch(const char *cmd)
{
#ifdef WIN32
	// Under win32 you can't just use "system" and background the process to easily launch
	// another executable - so we use spawn instead...
	_spawnl(_P_NOWAIT, cmd, cmd, NULL);
#else
	// On other systems, just use system to launch the executable
	char buf[PATH_MAX];
	snprintf(buf, PATH_MAX, "%s &", cmd);
	system(buf);
#endif
} // app_launch

/*******************************************************************************************/
int main(int argc, char *argv[])
{
	char launcher_path[PATH_MAX];
	char exe_path[PATH_MAX];
	// Find where the launcher app lives - we assume the test executables are in the same location
	int fail = get_app_path (launcher_path, PATH_MAX);
	if (fail) // couldn't get a valid path...
		return -1;

	// not all supported platfoms provide a dirname function - do a simplified version here
	strncpy(exe_path, launcher_path, PATH_MAX);
	// find the last dir sep (note that get_app_path uses '/' on win32 also)
	char *dirp = strrchr(exe_path, '/');
	dirp ++;   // first char after the dir slash
	*dirp = 0; // terminate the path, removing the executables basename

	strncat(exe_path, "mcast_tx", PATH_MAX);
	app_launch(exe_path); // launch the sender

	*dirp = 0; // terminate the path - again
	strncat(exe_path, "mcast_rx", PATH_MAX);
	app_launch(exe_path); // launch the receiver

	return 0;
}

/* end of file */
