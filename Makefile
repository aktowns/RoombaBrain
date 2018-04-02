PROJECT_NAME := RoombaBrain

include $(IDF_PATH)/make/project.mk

proto:
	flatcc -a rpc/rpc_response.fbs -o./main/rpc/ 
	flatcc -a rpc/rpc_request.fbs -o./main/rpc/
