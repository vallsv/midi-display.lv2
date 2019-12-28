/*
 */

#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/atom/util.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>

#include <stdbool.h>
#include <stdlib.h>

typedef enum {
    MIDI_CONTROL_CHANGE = 0xB0,
    MIDI_PROGRAM_CHANGE = 0xC0,
} MidiSpec;

typedef enum {
    SETBFREE_MIDI_RANDOM_DRAWBARS = 55,
} SetBFreeSettings;

typedef struct {
    LV2_Atom_Event event;
    uint8_t        msg[3];
} LV2_Atom_MIDI;

typedef enum {
    PORT_ATOM_IN = 0,

	PORT_CONTROL_FIRST = 1,
    PORT_CONTROL_RECEIVED = PORT_CONTROL_FIRST,

    PORT_CONTROL_MSG1,
    PORT_CONTROL_MSG2,
    PORT_CONTROL_MSG3,

    // Note: it have to be the last
    PORT_ENUM_SIZE
} PortEnum;

typedef struct {
    float* port;
    float last_value;
} Parameter;

typedef struct {
    // URIDs
    LV2_URID urid_midiEvent;

    // state
    // Note: it also alloc slots for MIDI in/out but we dont care
    Parameter parameters[PORT_ENUM_SIZE];

    // atom ports
    const LV2_Atom_Sequence* port_events_in;
    LV2_Atom_Sequence* port_events_out;

    // memory for MIDI message sending
    uint8_t msg_buffer[4 * PORT_ENUM_SIZE];
} Data;


static LV2_Handle instantiate(const LV2_Descriptor*     descriptor,
                              double                    rate,
                              const char*               path,
                              const LV2_Feature* const* features)
{
    Data* self = (Data*)calloc(1, sizeof(Data));

    for (int port = PORT_CONTROL_FIRST; port < PORT_ENUM_SIZE; port++) {
        Parameter *parameter = self->parameters + port;
        parameter->last_value = -1;
    }

    // Get host features
    LV2_URID_Map* urid_map = NULL;

    for (int i = 0; features[i]; ++i) {
        if (!strcmp(features[i]->URI, LV2_URID__map)) {
            urid_map = (LV2_URID_Map*)features[i]->data;
            break;
        }
    }

    if (!urid_map) {
        free(self);
        return NULL;
    }

    // Map URIs
    self->urid_midiEvent = urid_map->map(urid_map->handle, LV2_MIDI__MidiEvent);

    return self;
}

static void connect_port(LV2_Handle instance, uint32_t port, void* data)
{
    Data* self = (Data*)instance;

    switch (port)
    {
    case PORT_ATOM_IN:
            self->port_events_in = (const LV2_Atom_Sequence*)data;
            break;
    default:
        self->parameters[port].port = (float*)data;
    }
}

static void activate(LV2_Handle instance)
{
    // Data* self = (Data*)instance;
}

static void run(LV2_Handle instance, uint32_t sample_count)
{
    Data* self = (Data*)instance;
    const uint8_t *msg;

	/* process events on the midiin port */
	LV2_Atom_Event* ev = lv2_atom_sequence_begin(&(self->port_events_in)->body);
	while(!lv2_atom_sequence_is_end(&(self->port_events_in)->body, (self->port_events_in)->atom.size, ev)) {
		if (ev->body.type == self->urid_midiEvent) {
			msg = (const uint8_t*)(ev + 1);
			*self->parameters[PORT_CONTROL_MSG1].port = -1;
			*self->parameters[PORT_CONTROL_MSG2].port = -1;
			*self->parameters[PORT_CONTROL_MSG3].port = -1;
			for (uint8_t i = 0; i < ev->body.size; i++) {
				if (PORT_CONTROL_MSG1 + i > PORT_CONTROL_MSG3) {
					break;
				}
				*self->parameters[PORT_CONTROL_MSG1 + i].port = msg[i];
			}
		}
		ev = lv2_atom_sequence_next(ev);
	}
}

static void cleanup(LV2_Handle instance)
{
    free(instance);
}

static const LV2_Descriptor descriptor = {
    .URI = "https://github.com/vallsv/midi-display.lv2",
    .instantiate = instantiate,
    .connect_port = connect_port,
    .activate = activate,
    .run = run,
    .deactivate = NULL,
    .cleanup = cleanup,
    .extension_data = NULL
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    return (index == 0) ? &descriptor : NULL;
}
