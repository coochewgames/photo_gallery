cc -g -o photo_gallery photo_gallery.c photo_db.c set_config.c ../scene_handler/transition_handler.c -I../raylib/src -I../scene_handler -I/opt/vc/include -L../raylib/src -L/opt/vc/lib -lraylib -lm -lpthread -lGLESv2 -lEGL -lvcos -lvchiq_arm -lgbm -ldrm
