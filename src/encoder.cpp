#include <iostream>

#include "log.h"
#include "util.h"
#include "io.h"
#include "frame.h"
#include "frame_encode.h"
#include "frame_vlc.h"

Log logger("Main");

void *operator new(std::size_t n)
{
    // std::cerr << "[allocating " << n << " bytes]\n";
    return malloc(n);
}

void encode_sequence(Reader &reader, Writer &writer, Util &util)
{
    int curr_frame = 0;

    writer.write_sps(util.width, util.height, reader.nb_frames);
    writer.write_pps();
    auto frame_num = std::min(reader.nb_frames, util.test_frame == -1 ? reader.nb_frames : util.test_frame);
    while (curr_frame < frame_num)
    {
        Frame frame = reader.get_one_frame();

        logger.log(Level::NORMAL, "encode frame #" + std::to_string(curr_frame));

        encode_I_frame(frame);
        vlc_frame(frame);
        writer.write_slice(curr_frame, frame);

        curr_frame++;
    }
}

int main(int argc, const char *argv[])
{
    // Get command-line arguments
    Util util(argc, argv);

    // Read from given filename
    Reader reader(util.input_file, util.width, util.height);

    // Write to given filename
    Writer writer(util.output_file);

    // Encoding process start
    encode_sequence(reader, writer, util);

    return EXIT_SUCCESS;
}
