/* Test multicast operation and Fl::add_fd functionality */
/* This file is the multicast Transmitter - there should be one instance
 * of this running during the test. */

#ifdef WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  define  CLOSESKT	closesocket
#else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <netdb.h>
#  define  CLOSESKT	close
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Round_Button.H>

// define an arbitrary multicast address with organisation-local scope
static char madrs[] = "239.192.34.56";
#define SERVER_PORT 5678

static Fl_Double_Window *tx_win;
static Fl_Button *send_bt;
static Fl_Button *exit_bt;
static Fl_Round_Button *ready_bt;

static int skt;
static struct sockaddr_in servAddr;

/*******************************************************************************************/
static void cb_exit_bt(Fl_Button*, void*) {
 	tx_win->hide();
}

/*******************************************************************************************/
static void led_flash(void*) {
	static int led = 0;
	led = led ^ 1;
 	ready_bt->value(led);
 	Fl::repeat_timeout(0.4, led_flash);
}

/*******************************************************************************************/
static void send_cb(Fl_Button*, void*) { /* send some data */
	static int msg_idx = 0;
	char msg[128];
	for (int i = 0; i < 3; i++)
	{
		sprintf(msg, "Message text %d", msg_idx);
		msg_idx++;
		sendto(skt, msg, strlen(msg) + 1, 0, (struct sockaddr *)&servAddr, sizeof(servAddr));
	} /* end for */
}

/*******************************************************************************************/
int main (int argc, char *argv[])
{
	// First prepare the socket layer code
	struct sockaddr_in cliAddr;
	struct hostent *h;

#ifdef WIN32
	// On win32, make sure the winsock layer is started
    WSADATA WSAData;
    WSAStartup (MAKEWORD (1, 1), &WSAData);
#endif

	h = gethostbyname(madrs);
	if (h == NULL) {
		exit (-1);
	}

	// Populate the servAddr struct with the requested values
	servAddr.sin_family = h->h_addrtype;
	memcpy ((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
	servAddr.sin_port = htons (SERVER_PORT);

	/* create the socket */
	skt = socket (AF_INET, SOCK_DGRAM, 0);
	if (skt < 0) {
		fprintf(stderr, "cannot open socket\n");
		exit (-1);
	}

	/* bind port number */
	cliAddr.sin_family = AF_INET;
	cliAddr.sin_addr.s_addr = htonl(INADDR_ANY); // assume default eth_if is fine
	cliAddr.sin_port = htons (0);
	if (bind(skt, (struct sockaddr *)&cliAddr, sizeof(cliAddr)) < 0) {
		fprintf(stderr, "cannot bind port %d \n", SERVER_PORT);
		exit (-1);
	}

	/* set the IP TTL to 1, so our multicast datagrams can not get off the local network */
	char ttl = 1;
	setsockopt (skt, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

	// Now create the fltk window
	tx_win = new Fl_Double_Window(100, 100, 447, 338, "Sender");
	tx_win->begin();

	// msg send button
	send_bt = new Fl_Button(100, 100, 100, 50, "SEND");
	send_bt->box(FL_THIN_UP_BOX);
	send_bt->callback((Fl_Callback*)send_cb);

	// quit button
	exit_bt = new Fl_Button(360, 290, 64, 30, "Quit");
	exit_bt->box(FL_THIN_UP_BOX);
	exit_bt->callback((Fl_Callback*)cb_exit_bt);

	// flashing green "ready" led...
	ready_bt = new Fl_Round_Button(320, 20, 69, 30, "Ready");
	ready_bt->selection_color(FL_GREEN);
	ready_bt->when(FL_WHEN_NEVER);
	ready_bt->clear_visible_focus();

	tx_win->end();

	// display the Tx window
	tx_win->show(argc, argv);

	// Start the "ready led" flashing
	Fl::add_timeout(0.4, led_flash);

	// run the fltk core
	int res = Fl::run();

	/* close socket and exit */
	CLOSESKT(skt);
	return res;
}

/* End of File */
