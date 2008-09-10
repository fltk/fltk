/* Test multicast operation and Fl::add_fd functionality */
/* This file is the multicast Receiver - there can be multiple instances
 * of this running during the test. A minimum of one is good...! */

#ifdef WIN32
#  include <windows.h>
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  define	CLOSESKT	closesocket
#else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <netdb.h>
#  define	CLOSESKT	close
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Round_Button.H>

// define an arbitrary multicast address with organisation-local scope
static char madrs[] = "239.192.34.56";
#define SERVER_PORT 5678
#define MAX_MSG 100

static Fl_Double_Window *rx_win;
static Fl_Browser *rx_brws;
static Fl_Button *exit_bt;
static Fl_Round_Button *wait_bt;

/*******************************************************************************************/
static void cb_exit_bt(Fl_Button*, void*) {
 	rx_win->hide();
}

/*******************************************************************************************/
static void led_flash(void*) {
	static int led = 0;
	led = led ^ 1;
 	wait_bt->value(led);
 	Fl::repeat_timeout(0.4, led_flash);
}

/*******************************************************************************************/
static void skt_cb(int skt, void*) {
	struct sockaddr_in cliAddr;
	char msg[MAX_MSG];
	socklen_t cliLen = sizeof (cliAddr);
	int n = recvfrom (skt, msg, MAX_MSG, 0, (struct sockaddr *)&cliAddr, &cliLen);
	if (n < 0) return;
	rx_brws->add(msg);
	int last = rx_brws->size();
	rx_brws->bottomline(last);
}

/*******************************************************************************************/
int main (int argc, char *argv[])
{
	// First, create the socket layer
	int skt, res;
	struct ip_mreq mreq;
	struct sockaddr_in servAddr;
	struct in_addr mcastAddr;
	struct hostent *h;

#ifdef WIN32
	// On win32, make sure the winsock layer is started
    WSADATA WSAData;
    WSAStartup (MAKEWORD (1, 1), &WSAData);
#endif

	/* set mcast address to listen to */
	h = gethostbyname(madrs);
	if (h == NULL) {
		exit (-1);
	}

	memcpy(&mcastAddr, h->h_addr_list[0], h->h_length);

	/* create the socket */
	skt = socket (AF_INET, SOCK_DGRAM, 0);
	if (skt < 0) {
		fprintf (stderr, "cannot create socket\n");
		exit (-1);
	}

	/* set socket to allow re-use of local address before bind() - this should allow us to have
 	* multiple receiver instances all running at once. */
	int bReUse = 1;
	setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, (char *)&bReUse, sizeof(bReUse));

	// Populate the servAddr struct
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl (INADDR_ANY);
	servAddr.sin_port = htons (SERVER_PORT);

	/* bind port */
	if (bind (skt, (struct sockaddr *) &servAddr, sizeof (servAddr)) < 0) {
		fprintf(stderr, "cannot bind port %d \n", SERVER_PORT);
		exit (-1);
	}

	/* join multicast group */
	mreq.imr_multiaddr.s_addr = mcastAddr.s_addr;
	mreq.imr_interface.s_addr = htonl (INADDR_ANY);

	res = setsockopt(skt, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq));
	if (res != 0) {
		fprintf(stderr, "cannot join multicast group '%s'", inet_ntoa(mcastAddr));
		exit (-1);
	}

	// Now create the fltk display window
	rx_win = new Fl_Double_Window(600, 100, 447, 338, "Receiver");
	rx_win->begin();

	// A browser to display the rx'd text in
	rx_brws = new Fl_Browser(10, 10, 285, 310);
	rx_brws->when(FL_WHEN_NEVER);

	// quit button
	exit_bt = new Fl_Button(360, 290, 64, 30, "Quit");
	exit_bt->box(FL_THIN_UP_BOX);
	exit_bt->callback((Fl_Callback*)cb_exit_bt);

	// flashing red "waiting" led...
	wait_bt = new Fl_Round_Button(320, 20, 69, 30, "Waiting");
	wait_bt->selection_color(FL_RED);
	wait_bt->when(FL_WHEN_NEVER);
	wait_bt->clear_visible_focus();

	rx_win->end();

	// display the Rx window
	rx_win->show(argc, argv);

	// Start the "waiting led" flashing
	Fl::add_timeout(0.4, led_flash);

	// Add the socket to the add_fd check list
	Fl::add_fd(skt, skt_cb);

	// run the fltk core
	res = Fl::run();

	/* close socket and exit */
	CLOSESKT(skt);
	return res;
}

/* End of File */
