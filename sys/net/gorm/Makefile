MODULE = gorm

SRC = ll.c ll_trx.c util.c gorm.c

ifneq (,$(filter gorm_peripheral,$(USEMODULE)))
  SRC += ll_periph.c ll_chan.c ll_ctrl.c ll_host.c l2cap.c pdupool.c pduq.c
endif

ifneq (,$(filter gorm_gatt,$(USEMODULE)))
  SRC += uuid.c
  DIRS += $(RIOTBASE)/sys/net/gorm/gatt
endif

ifneq (,$(filter gorm_gap,$(USEMODULE)))
  DIRS += $(RIOTBASE)/sys/net/gorm/gap
endif

ifneq (,$(filter gorm_arch_riot,$(USEMODULE)))
  DIRS += $(RIOTBASE)/sys/net/gorm/arch/riot
endif

# include select build-in services
ifneq (,$(filter gorm_service_%,$(USEMODULE)))
  DIRS += $(RIOTBASE)/sys/net/gorm/service
endif

include $(RIOTBASE)/Makefile.base
