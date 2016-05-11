/* Copyright 2015 Martina Kollarova
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef ERROR_H
#define ERROR_H

#include <stdexcept>
#include <stdio.h>

void printGlErrors_(const char* where="", const int line=0);

#define printGlErrors() printGlErrors_(__FILE__, __LINE__)

#define fail(...) { fprintf(stderr, "[ERROR] (%s:%d) ", __FILE__, __LINE__); \
                    fprintf(stderr, __VA_ARGS__); \
                    exit(1); }

class Exception : public std::runtime_error
{
public:
    explicit Exception(const std::string& msg="")
        : runtime_error(msg) {}
};

#endif /* end of include guard: ERROR_H */
