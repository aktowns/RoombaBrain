PROJECT_NAME := RoombaBrain

include $(IDF_PATH)/make/project.mk

proto:
	protoc --plugin=protoc-gen-nanopb=./components/nanopb/generator/protoc-gen-nanopb rpc/request.proto \
  --nanopb_out=main/
	protoc --plugin=protoc-gen-nanopb=./components/nanopb/generator/protoc-gen-nanopb rpc/response.proto \
  --nanopb_out=main/
