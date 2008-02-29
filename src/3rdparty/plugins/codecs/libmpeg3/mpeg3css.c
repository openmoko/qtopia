#include "mpeg3css.h"
#include "mpeg3private.h"

extern "C" {

int mpeg3_init_css(mpeg3_t *, mpeg3_css_t *)
{
	return 0;
}

int mpeg3_get_keys(mpeg3_css_t *, char *)
{
	return 1;
}

int mpeg3_decrypt_packet(mpeg3_css_t *, unsigned char *)
{
	return 1;
}

mpeg3_css_t* mpeg3_new_css()
{
	return 0;
}

int mpeg3_delete_css(mpeg3_css_t *)
{
	return 0;
}

};

