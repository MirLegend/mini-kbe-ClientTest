// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#define INFO_MSG(Format, ...) \
{ \
    printf((Format), ##__VA_ARGS__);\
}

#define DEBUG_MSG(Format, ...) \
{ \
	printf((Format), ##__VA_ARGS__);\
}

#define WARNING_MSG(Format, ...) \
{ \
	printf((Format), ##__VA_ARGS__);\
}

#define SCREEN_WARNING_MSG(Format, ...) \
{ \
	printf((Format), ##__VA_ARGS__);\
}

#define ERROR_MSG(Format, ...) \
{ \
	printf((Format), ##__VA_ARGS__);\
}

#define SCREEN_ERROR_MSG(Format, ...) \
{ \
	printf((Format), ##__VA_ARGS__);\
}

#define SCREENDEBUG(Format, ...) \
{ \
	printf((Format), ##__VA_ARGS__);\
}