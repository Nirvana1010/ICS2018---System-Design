#include <am.h>
#include <x86.h>

#define RTC_PORT 0x48   // Note that this is not standard
static unsigned long boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
  return inl(RTC_PORT) - boot_time;
}

uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width  = 400,
  .height = 300,
};

extern void* memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
	/*
  int i;
  for (i = 0; i < _screen.width * _screen.height; i++) {
    fb[i] = i;
  }
	*/
	int len = sizeof(uint32_t) * ((x + w >= _screen.width) ? _screen.width - x : w);
  uint32_t *p = &fb[y * _screen.width + x];
	for(int i = 0; i < h; i++)
	{
		if(y + i < _screen.height)
				memcpy(p, pixels, len);
		else
			break;
    p += _screen.width;
		pixels += w;
	}
}

void _draw_sync() {
}

int _read_key() {
	uint32_t key_code = _KEY_NONE;
	if(inb(0x64) & 0x1)
		key_code = inl(0x60);

  return key_code;
}

void getScreen(int* width, int* height) {
	*width = _screen.width;
	*height = _screen.height;
}
