ROOT_VP_BUILD_DIR ?= $(CURDIR)/build
BUILD_DIR = $(ROOT_VP_BUILD_DIR)/launcher


-include $(PULP_SDK_HOME)/install/rules/vp_models.mk


CC = g++

CFLAGS +=  -MMD -MP -O2 -g -fpic -std=c++11 -Werror -Wall -I$(PULP_SDK_WS_INSTALL)/include
LDFLAGS += -O2 -g -shared -Werror -Wall -L$(PULP_SDK_WS_INSTALL)/lib -Wl,--whole-archive -ljson -Wl,--no-whole-archive

LAUNCHER_SRCS = src/launcher.cpp
LAUNCHER_OBJS = $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(patsubst src/%.c,$(BUILD_DIR)/%.o,$(LAUNCHER_SRCS)))

LAUNCHER_HEADERS += $(shell find include -name *.hpp)
LAUNCHER_HEADERS += $(shell find include -name *.h)

define declareInstallFile

$(PULP_SDK_WS_INSTALL)/$(1): $(1)
	install -D $(1) $$@

INSTALL_FILES += $(PULP_SDK_WS_INSTALL)/$(1)

endef

-include $(LAUNCHER_OBJS:.o=.d)

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(basename $@)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(basename $@)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD_DIR)/libpulpvplauncher.so: $(LAUNCHER_OBJS)
	@mkdir -p $(basename $@)
	$(CC) $^ -o $@ $(LDFLAGS)



$(foreach file, $(LAUNCHER_HEADERS), $(eval $(call declareInstallFile,$(file))))



$(PULP_SDK_WS_INSTALL)/lib/libpulpvplauncher.so: $(BUILD_DIR)/libpulpvplauncher.so
	install -D $^ $@


headers: $(INSTALL_FILES)

build: headers $(PULP_SDK_WS_INSTALL)/lib/libpulpvplauncher.so

clean:
	rm -rf $(BUILD_DIR)

.PHONY: build
