/*************************************************************************
 *
 * Filename:      databuffer.c
 * Version:       0.3
 * Description:   Network buffer handling routines.
 * Status:        Experimental.
 * Author:        Herton Ronaldo Krzesinski <herton@conectiva.com.br>
 * Created at:    Tue Jun 21 11:22:44 2005
 * Modified at:   Mon Aug 15 19:12:29 2005
 *
 *     Copyright (c) 2005 Herton Ronaldo Krzesinski, All Rights Reserved.
 *
 *     This library is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU Lesser General Public
 *     License as published by the Free Software Foundation; either
 *     version 2 of the License, or (at your option) any later version.
 *
 *     This library is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *     Lesser General Public License for more details.
 *
 *     You should have received a copy of the GNU Lesser General Public
 *     License along with this library; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *     MA  02111-1307  USA
 *
 *************************************************************************/

#include <databuffer.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

slist_t *slist_append(slist_t *list, void *element) 
{
	slist_t *node, *p;

	node = malloc(sizeof(slist_t));
	assert(node != NULL);
	node->data = element;
	node->next = NULL;
	if (!list)
		return node;
	p = list;
	while (p->next)
		p = p->next;
	p->next = node;
	return list;
}

slist_t *slist_remove(slist_t *list, void *element) 
{
	if (!list)
		return NULL;
	slist_t *prev, *next;
	prev = list;
	next = list;
	while (next != NULL) {
		if (next->data == element) {
			/* if first element, update list pointer */
			if (next == list) {
				list = list->next;
				prev = list;
				free(next);
				next = prev;
			} else {
				prev->next = next->next;
				free(next);
				next = prev->next;
			}
			continue;
		}
		prev = next;
		next = next->next;
	}
	return list;
}

buf_t *buf_new(size_t default_size) 
{
	buf_t *p;

	p = malloc(sizeof(buf_t));
	if (!p)
		return NULL;
	p->buffer = malloc(sizeof(uint8_t) * default_size);
	if (!p->buffer) {
		free(p);
		return NULL;
	}
	p->data = p->buffer;
	p->head_avail = 0;
	p->data_avail = default_size;
	p->tail_avail = 0;
	p->data_size = 0;
	return p;
}

size_t buf_total_size(buf_t *p) 
{
	if (!p)
		return 0;
	return p->head_avail + p->data_avail + p->tail_avail + p->data_size;
}

void buf_resize(buf_t *p, size_t new_size) 
{
	uint8_t *tmp;
	int bSize;

	if (!p)
		return;
	bSize = buf_total_size(p);
	if (new_size < bSize) {
		int itRem = bSize - new_size;
		if (itRem > p->data_avail) {
			itRem -= p->data_avail;
			p->data_avail = 0;
		} else {
			p->data_avail -= itRem;
			itRem = 0;
		}
		if (itRem > p->tail_avail) {
			itRem -= p->tail_avail;
			p->tail_avail = 0;
		} else {
			p->tail_avail -= itRem;
			itRem = 0;
		}
		/* When deallocating data from header we need to move
		 * the data used to the beginning after the header new
		 * allocated space, so we need to do memmove here */
		if (itRem > p->head_avail) {
			itRem -= p->head_avail;
			memmove(p->buffer, p->buffer + p->head_avail, p->data_size);
			p->head_avail = 0;
		} else {
			p->head_avail -= itRem;
			memmove(p->buffer + p->head_avail, p->buffer + p->head_avail + itRem, p->data_size);
			itRem = 0;
		}
		if (itRem > p->data_size) {
			p->data_size = 0;
		} else {
			p->data_size -= itRem;
		}
		bSize = 0;
	} else
		bSize = new_size - bSize;
	tmp = realloc(p->buffer, new_size);
	if (!new_size) {
		p->buffer = NULL;
		p->data = NULL;
		p->head_avail = 0;
		p->data_avail = 0;
		p->tail_avail = 0;
		p->data_size = 0;
		return;
	}
	if (!tmp)
		return;
	p->data_avail += bSize;
	p->buffer = tmp;
	p->data = p->buffer + p->head_avail;
}

buf_t *buf_reuse(buf_t *p) 
{
	if (!p)
		return NULL;
	p->data_avail = buf_total_size(p);
	p->head_avail = 0;
	p->tail_avail = 0;
	p->data_size = 0;
	p->data = p->buffer;
	return p;
}

void *buf_reserve_begin(buf_t *p, size_t data_size) 
{
	if (!p)
		return NULL;
	if (p->head_avail >= data_size) {
		p->head_avail -= data_size;
		p->data_size += data_size;
		p->data = p->buffer + p->head_avail;
		return p->buffer + p->head_avail;
	} else {
		if (data_size > p->head_avail + p->data_avail) {
			int tmp;
			tmp = p->data_avail;
			buf_resize(p, buf_total_size(p) + data_size -
			                    p->head_avail - p->data_avail);
			if (tmp == p->data_avail)
				return NULL;
			p->data_avail = 0;
		} else
			p->data_avail -= data_size - p->head_avail;
		memmove(p->buffer + data_size, p->buffer + p->head_avail, p->data_size);
		p->head_avail = 0;
		p->data = p->buffer;
		p->data_size += data_size;
		return p->buffer;
	}
}

void *buf_reserve_end(buf_t *p, size_t data_size) 
{
	if (!p)
		return NULL;
	void *t;

	if (p->tail_avail >= data_size)
		p->tail_avail -= data_size;
	else {
		if (data_size > p->tail_avail + p->data_avail) {
			int tmp;
			tmp = p->data_avail;
			buf_resize(p, buf_total_size(p) + data_size -
			                    p->tail_avail - p->data_avail);
			if (tmp == p->data_avail)
				return NULL;
			p->data_avail = 0;
		} else
			p->data_avail -= data_size - p->tail_avail;
		p->tail_avail = 0;
	}
	t = p->buffer + p->head_avail + p->data_size;
	p->data_size += data_size;
	p->data = p->buffer + p->head_avail;
	return t;
}

void buf_insert_begin(buf_t *p, uint8_t *data, size_t data_size) 
{
	uint8_t *dest;

	dest = (uint8_t *) buf_reserve_begin(p, data_size);
	assert(dest != NULL);
	memcpy(dest, data, data_size);
}

void buf_insert_end(buf_t *p, uint8_t *data, size_t data_size) 
{
	uint8_t *dest;

	dest = (uint8_t *) buf_reserve_end(p, data_size);
	assert(dest != NULL);
	memcpy(dest, data, data_size);
}

void buf_remove_begin(buf_t *p, size_t data_size) 
{
	if (!p)
		return;
	if (data_size < p->data_size) {
		p->head_avail += data_size;
		p->data_size -= data_size;
	} else {
		p->head_avail += p->data_size;
		p->data_size = 0;
	}
	p->data = p->buffer + p->head_avail;
}

void buf_remove_end(buf_t *p, size_t data_size) 
{
	if (!p)
		return;
	if (data_size < p->data_size) {
		p->tail_avail += data_size;
		p->data_size -= data_size;
	} else {
		p->tail_avail += p->data_size;
		p->data_size = 0;
	}
}

void buf_dump(buf_t *p, const char *label) 
{
	if (!p || !label)
		return;
	int i, n;

	n = 0;
	for (i = 0; i < p->data_size; ++i) {
#ifndef OBEX_SYSLOG
		if (n == 0)
			fprintf(stderr, "%s: ", label);
		fprintf(stderr, "%02X ", p->data[i]);
		if (n >= 25 || i == p->data_size - 1) {
			fprintf(stderr, "\n");
#else
		if (n == 0)
			syslog(LOG_DEBUG, "OpenObex: %s: ", label);
		syslog(LOG_DEBUG, "%02X ", p->data[i]);
		if (n >= 25 || i == p->data_size - 1) {
			syslog(LOG_DEBUG, "\n");
#endif
			n = -1;
		}
		n++;
	}
}

void buf_free(buf_t *p) 
{
	if (!p)
		return;
	if (p->buffer) {
		free(p->buffer);
	}
	free(p);
}
