# About

This application provides a test for the global event loop module. It will
simply post two custom events to the global event queue and make sure these
events are executed in the global event loop's thread context.
