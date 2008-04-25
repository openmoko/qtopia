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

#include "wordprediction.h"

#include "pred.h"
#include <QRect>

#define TO_PREDICT_STATE(handle) reinterpret_cast<PredictState*>(handle)
#define TO_WORD_PREDICT(handle)  reinterpret_cast<WordPredict*>(TO_PREDICT_STATE(handle))

struct PredictState {
    PredictState(const WordPredict::Config& config, int max_words)
        : predict(config, max_words)
    {}

    WordPredict predict;
    QString currentWord;
};

extern "C" {
word_prediction_handle word_prediction_init(int max_words, int swidth)
{
    WordPredict::Config config;
    config.reallyNoMoveSensitivity = swidth / 48;
    config.moveSensitivity = swidth / 12;
    config.excludeDistance = (swidth * 10) / 48;
    return new PredictState(config, max_words);
}

void word_prediction_free(word_prediction_handle handle)
{
    delete TO_PREDICT_STATE(handle);
}

void word_prediction_set_position(word_prediction_handle handle, char key, int x, int y, int width, int height)
{
    TO_WORD_PREDICT(handle)->setLetter(key, QRect(x, y, width, height));
}

void word_prediction_add_perfect_letter(word_prediction_handle handle, char* letter)
{
    QString string = QString::fromUtf8(letter);
    if (string.length() < 1)
        return;

    TO_WORD_PREDICT(handle)->addLetter(string[0].toLatin1());
}

void word_prediction_add_touch(word_prediction_handle handle, int x, int y)
{
    TO_WORD_PREDICT(handle)->addTouch(QPoint(x, y));
}

void  word_prediction_clear_word(word_prediction_handle handle)
{
    TO_WORD_PREDICT(handle)->reset();
}

void  word_prediction_get_number_words(word_prediction_handle handle)
{
    TO_WORD_PREDICT(handle)->words().count();
}

char* word_prediction_get_word(word_prediction_handle handle, int index)
{
    PredictState* state = TO_PREDICT_STATE(handle);
    state->currentWord = state->predict.words()[index];
    return state->currentWord.toUtf8().data();
}

float word_prediction_get_rate(word_prediction_handle handle, int index)
{
    return TO_WORD_PREDICT(handle)->ratings()[index];
}

char* word_prediction_get_best_word(word_prediction_handle handle)
{
    PredictState* state = TO_PREDICT_STATE(handle);
    state->currentWord = state->predict.word();
    return state->currentWord.toUtf8().data();
}

}
