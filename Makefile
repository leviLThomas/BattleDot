BUILD_DIR=build/

all: 
	$(MAKE) -C src all

clean:
	rm -f $(BUILD_DIR)client
	rm -f $(BUILD_DIR)server
