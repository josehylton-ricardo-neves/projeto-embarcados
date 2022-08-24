void init_all_components();

bool read_ac_voltage();

float read_temperature();

float read_dc_voltage();

float read_baterry_current();

bool check_ac_power_supply();

bool check_dc_power_supply();

void enable_ac_power_supply();

void disable_ac_power_supply();

void enable_dc_power_supply();

void disable_dc_power_supply();

bool check_battery_conditions();

void handle_mqtt_message(int tamanho, char * message);