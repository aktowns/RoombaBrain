PROJECT_NAME := RoombaBrain

include $(IDF_PATH)/make/project.mk

proto:
	protoc --plugin=protoc-gen-nanopb=./3p/nanopb/generator/protoc-gen-nanopb rpc/roomba.proto \
  --nanopb_out=main/
	cp -v {./3p/nanopb/pb.h,./3p/nanopb/pb_decode.c,./3p/nanopb/pb_encode.c} main/rpc/
