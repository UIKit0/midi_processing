#ifndef _MIDI_CONTAINER_H_
#define _MIDI_CONTAINER_H_

#include <cstdint>
#include <string>
#include <vector>

struct midi_event
{
	enum
	{
		max_static_data_count = 16
	};

	enum event_type
	{
		note_off = 0,
		note_on,
		polyphonic_aftertouch,
		control_change,
		program_change,
		channel_aftertouch,
		pitch_wheel,
		extended
	};

	unsigned m_timestamp;

	event_type m_type;
	unsigned m_channel;
	unsigned m_data_count;
    uint8_t m_data[max_static_data_count];
    std::vector<uint8_t> m_ext_data;

	midi_event() : m_timestamp(0), m_type(note_off), m_channel(0), m_data_count(0) { }
	midi_event( const midi_event & p_in );
    midi_event( unsigned p_timestamp, event_type p_type, unsigned p_channel, const uint8_t * p_data, std::size_t p_data_count );

	unsigned get_data_count() const;
    void copy_data( uint8_t * p_out, unsigned p_offset, unsigned p_count ) const;
};

class midi_track
{
    std::vector<midi_event> m_events;

public:
	midi_track() { }
	midi_track(const midi_track & p_in);

	void add_event( const midi_event & p_event );
    std::size_t get_count() const;
    const midi_event & operator [] ( std::size_t p_index ) const;
};

struct tempo_entry
{
	unsigned m_timestamp;
	unsigned m_tempo;

	tempo_entry() : m_timestamp(0), m_tempo(0) { }
	tempo_entry(unsigned p_timestamp, unsigned p_tempo);
};

class tempo_map
{
    std::vector<tempo_entry> m_entries;

public:
	void add_tempo( unsigned p_tempo, unsigned p_timestamp );
    unsigned timestamp_to_ms( unsigned p_timestamp, unsigned p_dtx ) const;

    std::size_t get_count() const;
    const tempo_entry & operator [] ( std::size_t p_index ) const;
};

struct system_exclusive_entry
{
    std::size_t m_port;
    std::size_t m_offset;
    std::size_t m_length;
	system_exclusive_entry() : m_port(0), m_offset(0), m_length(0) { }
	system_exclusive_entry(const system_exclusive_entry & p_in);
    system_exclusive_entry(std::size_t p_port, std::size_t p_offset, std::size_t p_length);
};

class system_exclusive_table
{
    std::vector<uint8_t> m_data;
    std::vector<system_exclusive_entry> m_entries;

public:
    unsigned add_entry( const uint8_t * p_data, std::size_t p_size, std::size_t p_port );
    void get_entry( unsigned p_index, const uint8_t * & p_data, std::size_t & p_size, std::size_t & p_port );
};

struct midi_stream_event
{
	unsigned m_timestamp;
	unsigned m_event;

	midi_stream_event() : m_timestamp(0), m_event(0) { }
	midi_stream_event(unsigned p_timestamp, unsigned p_event);
};

struct midi_meta_data_item
{
	unsigned m_timestamp;
    std::string m_name;
    std::string m_value;

	midi_meta_data_item() : m_timestamp(0) { }
	midi_meta_data_item(const midi_meta_data_item & p_in);
	midi_meta_data_item(unsigned p_timestamp, const char * p_name, const char * p_value);
};

class midi_meta_data
{
    std::vector<midi_meta_data_item> m_data;

public:
	midi_meta_data() { }

	void add_item( const midi_meta_data_item & p_item );

	void append( const midi_meta_data & p_data );
	
	bool get_item( const char * p_name, midi_meta_data_item & p_out ) const;

    std::size_t get_count() const;

    const midi_meta_data_item & operator [] ( std::size_t p_index ) const;
};

class midi_container
{
public:
	enum
	{
		clean_flag_emidi       = 1 << 0,
		clean_flag_instruments = 1 << 1,
		clean_flag_banks       = 1 << 2,
	};

private:
	unsigned m_form;
	unsigned m_dtx;
    std::vector<uint64_t> m_channel_mask;
    std::vector<tempo_map> m_tempo_map;
    std::vector<midi_track> m_tracks;

    std::vector< std::vector< std::string > > m_device_names;

	midi_meta_data m_extra_meta_data;

    std::vector<unsigned> m_timestamp_end;

    std::vector<unsigned> m_timestamp_loop_start;
    std::vector<unsigned> m_timestamp_loop_end;

    unsigned timestamp_to_ms( unsigned p_timestamp, unsigned p_subsong ) const;

public:
    midi_container() { m_device_names.resize( 16 ); }

	void initialize( unsigned p_form, unsigned p_dtx );

	void add_track( const midi_track & p_track );

    void add_track_event( std::size_t p_track_index, const midi_event & p_event );

	void set_extra_meta_data( const midi_meta_data & p_data );

    void serialize_as_stream( unsigned subsong, std::vector<midi_stream_event> & p_stream, system_exclusive_table & p_system_exclusive, unsigned clean_flags ) const;

    void serialize_as_standard_midi_file( std::vector<uint8_t> & p_midi_file ) const;

    unsigned get_subsong_count() const;
    unsigned get_subsong( unsigned p_index ) const;

    unsigned get_timestamp_end(unsigned subsong, bool ms = false) const;

    unsigned get_format() const;
    unsigned get_track_count() const;
    unsigned get_channel_count(unsigned subsong) const;

    unsigned get_timestamp_loop_start(unsigned subsong, bool ms = false) const;
    unsigned get_timestamp_loop_end(unsigned subsong, bool ms = false) const;

	void get_meta_data( unsigned subsong, midi_meta_data & p_out );

	void scan_for_loops( bool p_xmi_loops, bool p_marker_loops );

    static void encode_delta( std::vector<uint8_t> & p_out, unsigned delta );
};

#endif