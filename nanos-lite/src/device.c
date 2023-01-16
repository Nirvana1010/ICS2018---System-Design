#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

int current_game = 0;
size_t events_read(void *buf, size_t len) {
	char str[20];
	bool down = false;
	int key = _read_key();
	if(key & 0x8000) {
		key ^= 0x8000;
		down = true;
	}
	
	if(key != _KEY_NONE) {
	 sprintf(str, "%s %s\n", down ? "kd" : "ku", keyname[key]);
	 
	 //F12
	 if(key == 13 && down) {
	   current_game = (current_game == 0 ? 1 : 0);
	 }
	}
	else 
		sprintf(str, "t %d\n", _uptime());
		
	if(strlen(str) <= len) {
		strncpy((char*)buf, str, strlen(str));
		return strlen(str);
	}
	Log("strlen(event) > len, return 0");
  return 0;
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
	strncpy(buf, dispinfo+offset, len);
}

extern void getScreen(int *width, int* height);
void fb_write(const void *buf, off_t offset, size_t len) {
	assert(offset%4 == 0 && len%4 == 0);
	int index, screen_x, screen_y;
	int width = 0, height = 0;
	getScreen(&width, &height);
	
	for(int i = 0; i < len/4; i++) {
		index = offset/4 + i;
		screen_y = index/width;
		screen_x = index % width;
		_draw_rect(buf+i*4, screen_x, screen_y, 1, 1);
	}
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  int width = 0, height = 0;
	getScreen(&width, &height);
	sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", width, height);
}
