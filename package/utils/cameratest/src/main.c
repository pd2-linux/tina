#include "hawkview.h"

int main()
{
	hawkview_handle *hawkview;

	hawkview_init(&hawkview);
	//printf("capture set_w %d\n",hawkview->capture.set_w);
	//printf("capture set_h %d\n",hawkview->capture.set_h);
	hawkview_start(hawkview);

	hawkview_stop(hawkview);

	hawkview_release(hawkview);

	return 0;
}
