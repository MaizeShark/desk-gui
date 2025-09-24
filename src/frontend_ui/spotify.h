// src/frontend_ui/spotify.h

#ifndef SPOTIFY_H
#define SPOTIFY_H

#include "config.h"

/**
 * @brief Initializes Spotify authentication and connection.
 */
void spotify_setup();

/**
 * @brief Test function to print current playback state to Serial.
 */
void test_spotify();

#endif // SPOTIFY_H