#===============================================================================
#Autor: 
#   Ezequiel Donhauser
#Descricao:
#   Script de compilação do codigo fonte
#===============================================================================

#nome da toolchain (ferramentas de compilação)
TOOLCHAIN ?= C:\ToolChains\GNU_XTENSA\bin\xtensa-lx106-elf

#opçoes de compilação para arquivos individualmente (gcc)
GCC_OPTIONS=-Os -g -O2 -Wpointer-arith -Wundef -Werror -Wl,-EL		    \
	    -fno-inline-functions -nostdlib -mlongcalls			    \
	    -mtext-section-literals  -D__ets__ -DICACHE_FLASH	    

#opçoes de compilação para objeto final (linker)
LD_OPTIONS =-nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static \
	    -Wl,--start-group -lc -lgcc -lhal -lphy -lpp -lnet80211 -llwip   \
	    -lwpa -lmain build/app_app.a -Wl,--end-group

#Bibliotecas e ferramentas que serão incluidas
SDK_TOOLS=C:\Projetos\ESP8266\ESP_PROJECT\utils
GCC_LIBS=-LC:\Projetos\ESP8266\ESP_PROJECT\lib
LD_LIBS=c gcc hal phy pp net80211 lwip wpa main
LD_SCRIPT=-TC:\Projetos\ESP8266\ESP_PROJECT\script.ld

#insira aqui os objetos para serem gerados (corespondem a cada arquivo .c)
OBJECT_FILES=user_main.o uart.o

#inicia processo de compilação aqui
all: $(OBJECT_FILES)
	$(TOOLCHAIN)-ar cru build/app_app.a build/uart.o build/user_main.o
	$(TOOLCHAIN)-gcc $(GCC_LIBS) $(LD_SCRIPT) $(LD_OPTIONS) -o build/app.out
	
	@echo " "
	@$(SDK_TOOLS)\memanalyzer.exe $(TOOLCHAIN)-objdump.exe build/app.out
	@echo " "
	@$(TOOLCHAIN)-objcopy --only-section .text -O binary build/app.out eagle.app.v6.text.bin
	@$(TOOLCHAIN)-objcopy --only-section .data -O binary build/app.out eagle.app.v6.data.bin
	@$(TOOLCHAIN)-objcopy --only-section .rodata -O binary build/app.out eagle.app.v6.rodata.bin
	@$(TOOLCHAIN)-objcopy --only-section .irom0.text -O binary build/app.out eagle.app.v6.irom0text.bin
	@$(SDK_TOOLS)/gen_appbin.exe build/app.out 0 0 0 0
	@mv eagle.app.flash.bin build/file_0x00000.bin
	@mv eagle.app.v6.irom0text.bin build/file_0x40000.bin
	@rm -f eagle.app.v6.data.bin
	@rm -f eagle.app.v6.irom0text.bin
	@rm -f eagle.app.v6.rodata.bin
	@rm -f eagle.app.v6.text.bin
	@echo "file: build\file_0x00000.bin"
	@echo "file: build\file_0x40000.bin"
	
# $(TOOLCHAIN)-objdump -h -j .data -j .rodata -j .bss -j .text -j .irom0.text build/app.out
	
#remove todos os arquivos gerados pela compilação
clean:
	rm -f build/*.o
	rm -f build/*.a
	rm -f build/*.out
	rm -f eagle.app.v6.data.bin
	rm -f eagle.app.v6.irom0text.bin
	rm -f eagle.app.v6.rodata.bin
	rm -f eagle.app.v6.text.bin

#--------------------------- user_main.c ---------------------------------------
user_main.o:
	$(TOOLCHAIN)-gcc $(GCC_OPTIONS) -c user_main.c -o build\user_main.o

#--------------------------- uart.c --------------------------------------------
uart.o:
	$(TOOLCHAIN)-gcc $(GCC_OPTIONS) -c uart.c -o build\uart.o
