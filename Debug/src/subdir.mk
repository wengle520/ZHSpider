################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/cJSON.c \
../src/epoll-example.c \
../src/spider.c \
../src/spider_no_epoll.c \
../src/sslayer.c \
../src/user.c 

OBJS += \
./src/cJSON.o \
./src/epoll-example.o \
./src/spider.o \
./src/spider_no_epoll.o \
./src/sslayer.o \
./src/user.o 

C_DEPS += \
./src/cJSON.d \
./src/epoll-example.d \
./src/spider.d \
./src/spider_no_epoll.d \
./src/sslayer.d \
./src/user.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


