/***************************************************************************
**
** Copyright (C) 2008 Openmoko Inc.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2 or at your option any later version.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************/

#ifndef OPENMOKO_WORD_PREDICTION_H
#define OPENMOKO_WORD_PREDICTION_H

extern "C" {

#define MOVEMENT_NONE                   0x0
#define MOVEMENT_UP                     0x1
#define MOVEMENT_DOWN                   0x2
#define MOVEMENT_LEFT                   0x4
#define MOVEMENT_RIGHT                  0x8
#define MOVEMENT_PERFECT                0x10

/**
 * Handle to the internal data structure
 */
typedef void* word_prediction_handle;

/**
 * Create and initialize a handle, specify some
 * configuration.
 */
word_prediction_handle* word_prediction_init();
void word_prediction_free(word_prediction_handle* handle);

/**
 * Where is the center of the @param key? This must be properly initialized for
 * every key used by the keyboard layout.
 */
void word_prediction_set_center(word_prediction_handle* handle, char key, int x, int y);

/**
 * Add the letter to the prediction a perfect hit at the position of @param letter
 * is assumed.
 */
void word_prediction_add_perfect_letter(word_prediction_handle* handle, char letter);

/**
 * Add a touch and describe the movement from the previos format. The movement has to
 * be NONE for the first charachter
 */
void word_prediction_add_touch(word_prediction_handle* handle, int x, int y, int movement);

/**
 * Call this to start a new word.
 */
void  word_prediction_clear_word(word_prediction_handle* handle);

/**
 * Get the number of predicted words. This can be used to iterate over
 * the list of words and their ratings.
 */
void  word_prediction_get_number_words(word_prediction_handle*);
char* word_prediction_get_word(word_prediction_handle*, int index);
int   word_prediction_get_rate(word_prediction_handle*, int index);
char* word_prediction_get_best_word(word_prediction_handle*);



};

#endif
