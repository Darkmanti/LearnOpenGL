// define for compiler
#define _CRT_SECURE_NO_WARNINGS

// Defines for additional libraries
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION

// Define some useful macros
#define DEFINE_TO_WSTR(X) L#X
#define WSTR_CONCAT_MESSAGE_AND_NUMBER(X, Y) X DEFINE_TO_WSTR(Y)

#define Kilobytes(value) ((value)*1024LL)
#define Megabytes(value) (Kilobytes(value)*1024LL)
#define Gigabytes(value) (Megabytes(value)*1024LL)
#define Terabytes(value) (Gigabytes(value)*1024LL)

#include "vendors/glad/src/glad.c"
#include "main.cpp"