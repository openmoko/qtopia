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

/**
 * Handle to the internal data structure
 */
typedef void* word_prediction_handle;

/**
 * Create and initialize a handle, specify some
 * configuration.
 */
word_prediction_handle word_prediction_init(int max_words, int width);
void word_prediction_free(word_prediction_handle handle);

/**
 * Where is the center of the @param key? This must be properly initialized for
 * every key used by the keyboard layout.
 */
void word_prediction_set_position(word_prediction_handle handle, char key, int x, int y, int width, int height);

/**
 * Add the letter to the prediction a perfect hit at the position of @param letter
 * is assumed.
 */
void word_prediction_add_perfect_letter(word_prediction_handle handle, char letter);

/**
 * Add a touch and describe the movement from the previos format. The movement has to
 * be NONE for the first charachter.
 */
void word_prediction_add_touch(word_prediction_handle handle, int x, int y);

/**
 * Call this to start a new word.
 */
void  word_prediction_clear_word(word_prediction_handle handle);

/**
 * Get the number of predicted words. This can be used to iterate over
 * the list of words and their ratings.
 */
void  word_prediction_get_number_words(word_prediction_handle handle);

/**
 * Returns the word in UTF-8. The pointer is valid until the next call to
 * get_best_word, get_word or _reset.
 */
char* word_prediction_get_word(word_prediction_handle handle, int index);
float word_prediction_get_rate(word_prediction_handle handle, int index);
char* word_prediction_get_best_word(word_prediction_handle handle);



};

#endif
