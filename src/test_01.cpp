#include <dlfcn.h>
#include <signal.h>
#include <stdio.h>



#define SUNVOX_MAIN
#include <sunvox.h>

int keep_running = 1;
void int_handler( int param ) 
{
    keep_running = 0;
}

int g_sv_sampling_rate = 44100; //Hz
int g_sv_channels_num = 2; //1 - mono; 2 - stereo
int g_sv_buffer_size = 1024 * 4; //Audio buffer size (number of frames)
int g_sv_slot = 0;

static int init(){
    signal(SIGINT, int_handler);
    if(sv_load_dll())
        return -1;
    int ver = sv_init(
        0,                          // config
        g_sv_sampling_rate,         // freq
        g_sv_channels_num,          // channel
        SV_INIT_FLAG_USER_AUDIO_CALLBACK | SV_INIT_FLAG_AUDIO_INT16 | SV_INIT_FLAG_ONE_THREAD
        );
    return ver;
}
static void dump_version(int version){
    int major = ( version >> 16 ) & 255;
	int minor1 = ( version >> 8 ) & 255;
	int minor2 = ( version ) & 255;
    printf( "SunVox lib version: %d.%d.%d\n", major, minor1, minor2 );
}

static void dump_module(int mod){
    unsigned int module_flags = sv_get_module_flags(g_sv_slot, mod);
    int num_inputs = ( module_flags & SV_MODULE_INPUTS_MASK ) >> SV_MODULE_INPUTS_OFF;
    int num_outputs = ( module_flags & SV_MODULE_OUTPUTS_MASK ) >> SV_MODULE_OUTPUTS_OFF;
    const char * module_name = sv_get_module_name(g_sv_slot, mod);
    printf("Module name:%s, inputs:%d, outputs:%d\n", module_name, num_inputs, num_outputs);
    int num_ctls = sv_get_number_of_module_ctls(g_sv_slot, mod);
    printf("Controlers:\n");
    for(int i=0;i<num_ctls;i++){
        const char * ctl_name = sv_get_module_ctl_name(g_sv_slot, mod, i);
        int crl_value = sv_get_module_ctl_value(g_sv_slot, mod, i, 0);
        printf("  %-14s: %d\n", ctl_name, crl_value);
    }
}

static void dump_slot(){
    int num_mod = sv_get_number_of_modules(g_sv_slot);
    int num_ptn = sv_get_number_of_patterns(g_sv_slot);
    unsigned int tps = sv_get_ticks_per_second();
    printf("Current slot, %d pattern, %d modules, tps:%u\n", num_ptn, num_mod, tps);
    printf("--------------------------------------\n");
    for(int i=0;i<num_mod;i++){
        dump_module(i);
        printf("--------------------------------------\n");
    }
}

int main(){
    int version = init();
    if(version < 0)
        return -1;
    dump_version(version);

    // open a slot, which contains a signle instance of sunvox engine
    sv_open_slot(g_sv_slot);

    sv_volume( g_sv_slot, 256 );
	sv_lock_slot( g_sv_slot );
    int mod_gen_1 = sv_new_module( g_sv_slot, "Generator", "gen_1", 10, 0, 0 );

    

    int mod_gen_2 = sv_new_module( g_sv_slot, "Generator", "gen_2", 20, 0, 0 );
    int mod_echo = sv_new_module(g_sv_slot, "Echo", "echo_1", 30, 0, 0);
    sv_connect_module( g_sv_slot, mod_gen_1, 0 );
    sv_connect_module( g_sv_slot, mod_gen_2, mod_echo );
    sv_connect_module( g_sv_slot, mod_echo, 0 );
    sv_unlock_slot( g_sv_slot);


    // modify module ctl value, no need to lock
    sv_send_event( g_sv_slot, 0, 0, 0, mod_gen_1+1, 7<<8/*Mode*/, 1 );
    sv_play_from_beginning( g_sv_slot); // let modify take effect

    dump_slot();


    sv_close_slot( g_sv_slot);

	sv_deinit();
    
    sv_unload_dll();

    return 0;
}