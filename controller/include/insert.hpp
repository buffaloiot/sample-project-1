#pragma once

/**
 * DB Insert Headers
 */

#include "config.hpp"
#include <ctime>
#include <iostream>
#include <pqxx/pqxx>

extern void insert_reading(appConfig*, const char*, const char*, const char*, const char*, int);
