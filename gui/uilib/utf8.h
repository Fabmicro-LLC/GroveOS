#ifndef _UTF8_H_
#define _UTF8_H_

#include <stdint.h>
#include <string.h>

uint32_t u8_nextchar(const char *s, int *i);
int u8_strlen(const char *s);
char* u8_substr(char* dst, const char *s, int start, int len);
int u8_index(const char *s, int start);
void u8_erase(char *s, int start, int end);
void u8_insert(char *s, int s_size, int start, char* text);
char* u8_to_win1251(const char* utf8, char* win1251, int win1251_len);

#endif //_UTF8_H_

#ifdef UTF8_IMPL

#ifndef _UTF8_H_IMPL_
#define _UTF8_H_IMPL_

#include <string.h>

/* is c the start of a utf8 sequence? */
#define isutf(c) (((c)&0xC0)!=0x80)

static const uint32_t offsetsFromUTF8[6] = {
    0x00000000UL, 0x00003080UL, 0x000E2080UL,
    0x03C82080UL, 0xFA082080UL, 0x82082080UL
};

/* reads the next utf-8 sequence out of a string, updating an index */
uint32_t u8_nextchar(const char *s, int *i) {
    uint32_t ch = 0;
    int sz = 0;

    do {
        ch <<= 6;
        ch += (unsigned char)s[(*i)++];
        sz++;
    } while (s[*i] && !isutf(s[*i]));
    ch -= offsetsFromUTF8[sz-1];

    return ch;
}

/* number of characters */
int u8_strlen(const char *s) {
    int count = 0;
    int i = 0;

    if(s[0] == 0) return 0;

    while (u8_nextchar(s, &i) != 0) count++;

    return count;
}

char* u8_substr(char* dst, const char *s, int start, int len) {
	//dst should be the same size as src

	if(len<=0) {
		dst[0] = 0;
		return dst;
	}

	int count = 0;
    	int i = 0;
	uint32_t c = 0;
	int index_start = 0;

	if(start<0) start = 0;

    	while ((c = u8_nextchar(s, &i)) != 0) {
		count++;

		if(count == start) {
			index_start = i;	
		}

		if(count == start+len) {
			break;
		}

	}

	memmove(dst, s+index_start, (i-index_start));
	dst[i-index_start] = 0;
	return dst;
}

int u8_index(const char *s, int start) {
        int count = 0;
        int i = 0;
        uint32_t c = 0;

        if(start<=0) return 0;

        while ((c = u8_nextchar(s, &i)) != 0) {
                count++;
                if(count == start) break;
        }

        return i;
}

void u8_erase(char *s, int start, int end) {
	int clen = strlen(s);
	if(clen>0) {
        	int pos1 = u8_index(s, start);
        	int pos2 = u8_index(s, end);
        	memmove(s + pos1, s + pos2, clen - pos2);
        	s[clen - (pos2-pos1)] = 0;
	}
}

void u8_insert(char *s, int s_size, int start, char* text) {
	int clen = strlen(s);
        int text_clen = strlen(text);

	if(s_size < clen + text_clen) return;

        int pos = u8_index(s, start);
        memmove(s+pos+text_clen, s + pos, clen - pos);
        memmove(s+pos, text, text_clen);
	s[clen + text_clen] = 0;		
}



char* u8_to_win1251(const char* utf8, char* win1251, int win1251_len) {
	int i = 0;
	int j = 0;
	uint32_t c = 0;
    	while ((c = u8_nextchar(utf8, &i)) != 0 && j<win1251_len-1) {
		if ( c >= 0x410 && c <= 0x44F ) {
                	win1251[j] = (char)(c - 0x350);
                } else if (c >= 0x80 && c <= 0xFF) {
                	win1251[j] = (char)(c);	
		} else {
			win1251[j] = '?';
		}

		j++;
	}

	if(j<win1251_len) win1251[j] = 0;
	return win1251;
}

#endif //_UTF8_H_IMPL_
#endif //UTF8_IMPL
