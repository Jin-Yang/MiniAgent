
ADD_LIBRARY(log STATIC
	log.h common.c
	rotater.c rotater.h
	process.c thread.c stdout.c
)
TARGET_LINK_LIBRARIES(log pthread)
