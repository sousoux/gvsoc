ROOT_VP_BUILD_DIR ?= $(CURDIR)/build

PULP_PROPERTIES += udma/version timer/version stdout/version soc_eu/version \
	fc_itc/version icache_ctrl/version xne/version fll/version event_unit/version \
	cluster_ctrl/version pulp_chip soc/cluster dma/version udma/spim/version \
	udma/cpi/version padframe/version pulp_chip_family pmu/version \
	udma/hyper/version

properties := $(foreach prop,$(PULP_PROPERTIES), --property=$(prop))


VP_DIRS=memory pulp pulp/fll pulp/stdout pulp/chips/pulpissimo cpu/iss \
  interco utils pulp/itc pulp/hwpe/example pulp/timer pulp/soc_eu pulp/udma \
  pulp/hwpe/xne pulp/icache_ctrl pulp/cluster pulp/event_unit \
  pulp/chips/pulp pulp/mchan pulp/padframe pulp/adv_dbg_unit \
  pulp/chips/oprecompkw pulp/chips/oprecompkw_sa pulp/chips/bigpulp \
  pulp/chips/wolfe pulp/pmu pulp/chips/gap pulp/chips/multino \
  devices/hyperchip


-include $(ROOT_VP_BUILD_DIR)/props.mk

-include $(PULP_SDK_HOME)/install/rules/vp_models.mk

props:
	plpinfo mkgen --makefile=$(ROOT_VP_BUILD_DIR)/props.mk $(properties)

build: vp_build

clean: vp_clean

.PHONY: clean build props
