#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "freertos/freeRTOS.h"
#include "freertos/task.h"

#include "all_components_interface.h"

void app_main(void) {

    init_all_components();

	while(1){

		if(read_ac_voltage()){
			//vetifica se bateria ta ativada, se sim, desativa
			if(check_dc_power_supply()){
				disable_dc_power_supply();
			}
			//verifica se driver da rede não ta ativa, se nao tiver entao ativa rede
			if(!check_ac_power_supply()){
				enable_ac_power_supply();
			}
		//se nao existir tensao na rede
		}else {
			//se driver da rede tiver ativado desativa
			if(check_ac_power_supply()){
				disable_ac_power_supply();
			}
			//verifica condições da bateria para ligar a mesma
			if(check_battery_conditions()) {
				//se condições obedecidas e rele bateria estiver desativado entao ativa
				if(!check_dc_power_supply()){
					enable_dc_power_supply();
				}
			//se condições da bateria nao tiver ok verifica se driver da bateria ta ativo e se tiver desliga
			}else if(!check_battery_conditions()){
				if(check_dc_power_supply()){
					disable_dc_power_supply();
				}
			}
		}
		vTaskDelay(50 / portTICK_PERIOD_MS);
	}
}
