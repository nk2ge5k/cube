ROOT_DIR   = $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
BUILD_DIR  = $(ROOT_DIR)/build
CMAKE_MAKE = $(ROOT_DIR)/build/Makefile

CMAKE     ?= cmake

$(CMAKE_MAKE):
	mkdir -p $(BUILD_DIR) && $(CMAKE) -B $(BUILD_DIR) -S $(ROOT_DIR)

build: $(CMAKE_MAKE) ## Build binary
	@cd $(BUILD_DIR) && make -j 4 1>&2
.PHONY: build

help: ## Show this help
	@echo "\nSpecify a command. The choices are:\n"
	@grep -hE '^[0-9a-zA-Z_-]+:.*?## .*$$' ${MAKEFILE_LIST} | awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[0;36m%-20s\033[m %s\n", $$1, $$2}'
	@echo ""
.PHONY: help

.DEFAULT_GOAL := build
