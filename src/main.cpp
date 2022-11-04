/******************************************************************************
 * Spine Runtimes License Agreement
 * Last updated January 1, 2020. Replaces all prior versions.
 *
 * Copyright (c) 2013-2020, Esoteric Software LLC
 *
 * Integration of the Spine Runtimes into software or otherwise creating
 * derivative works of the Spine Runtimes is permitted under the terms and
 * conditions of Section 2 of the Spine Editor License Agreement:
 * http://esotericsoftware.com/spine-editor-license
 *
 * Otherwise, it is permitted to integrate the Spine Runtimes into software
 * or otherwise create derivative works of the Spine Runtimes (collectively,
 * "Products"), provided that each user of the Products must obtain their own
 * Spine Editor license and redistribution of the Products in any form must
 * include this license and copyright notice.
 *
 * THE SPINE RUNTIMES ARE PROVIDED BY ESOTERIC SOFTWARE LLC "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ESOTERIC SOFTWARE LLC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES,
 * BUSINESS INTERRUPTION, OR LOSS OF USE, DATA, OR PROFITS) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THE SPINE RUNTIMES, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

/**
 * @file main.cpp
 * @author Capybaras Country Club team (capybaras.country.club@gmail.com)
 * @brief A fully animated NFT collection living in the ETH blockchain
 * @version 1.0
 * @date 2022-04-04
 *
 * @copyright Capybaras Country Club (c) 2022
 *
 */

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <boost/program_options.hpp>
#include <fstream>
#include <gif_manager.h>
#include <iostream>
#include <json.hpp>
#include <spine/Debug.h>
#include <spine/Log.h>
#include <spine/spine-sfml.h>

using namespace std;
using namespace spine;
using namespace nlohmann;
#include <memory>

// Parameters
std::string metadata_info = "../metadata/_metadata.json";
std::string output_path = "../results/gif/";
std::string data_path = "../runtime_data";
std::string project_name = "chiguire_merge";
std::string scale_format = "0.3";
bool animation_completed = false;
bool verify_collection = false;
bool custom_collection = false;
bool debug_attributes = false;
bool gif_collection = false;
bool img_collection = false;
bool gif_started = false;
bool merge_gifs = false;
int initial_iteration = 0;
int final_iteration = 7700;
size_t frames_per_gif = 0;
float SCALE = 0.3;
float IMAGE_SIZE = 2000 * SCALE;
float delta_offset = 0;

// Parameters

template <typename T, typename... Args>
unique_ptr<T> make_unique_test(Args &&...args)
{
    return unique_ptr<T>(new T(forward<Args>(args)...));
}

class Attributes
{
public:
    std::string trait_type;
    std::string value;
    const std::string getFullName() { return trait_type + string("/") + value; };

    Attributes(std::string _trait_type, std::string _value)
        : trait_type(_trait_type), value(_value){};
};

void callback(AnimationState *state, EventType type, TrackEntry *entry, Event *event)
{
    SP_UNUSED(state);
    const String &animationName =
        (entry && entry->getAnimation()) ? entry->getAnimation()->getName() : String("");

    switch (type)
    {
    case EventType_Start:
        printf("%d start: %s\n", entry->getTrackIndex(), animationName.buffer());
        break;
    case EventType_Interrupt:
        printf("%d interrupt: %s\n", entry->getTrackIndex(), animationName.buffer());
        break;
    case EventType_End:
        printf("%d end: %s\n", entry->getTrackIndex(), animationName.buffer());
        break;
    case EventType_Complete:
        // printf("%d complete: %s\n", entry->getTrackIndex(), animationName.buffer());
        animation_completed = true;
        break;
    case EventType_Event:
        printf("%d event: %s, %s: %d, %f, %s %f %f\n",
               entry->getTrackIndex(),
               animationName.buffer(),
               event->getData().getName().buffer(),
               event->getIntValue(),
               event->getFloatValue(),
               event->getStringValue().buffer(),
               event->getVolume(),
               event->getBalance());
        break;
    }
    fflush(stdout);
}

shared_ptr<SkeletonData> readSkeletonBinaryData(const char *filename, Atlas *atlas, float scale_)
{
    SkeletonBinary binary(atlas);
    binary.setScale(scale_);
    auto skeletonData = binary.readSkeletonDataFile(filename);
    if (!skeletonData)
    {
        printf("%s\n", binary.getError().buffer());
        exit(0);
    }
    return shared_ptr<SkeletonData>(skeletonData);
}

void createGifCollection(void func(SkeletonData *skeletonData, Atlas *atlas),
                         const char *binaryName,
                         const char *atlasName,
                         float scale_)
{
    SFMLTextureLoader textureLoader;
    auto atlas = make_unique_test<Atlas>(atlasName, &textureLoader);

    auto skeletonData = readSkeletonBinaryData(binaryName, atlas.get(), scale_);
    func(skeletonData.get(), atlas.get());
}

string getString(string str)
{
    str.pop_back();
    str.erase(str.begin());
    return str;
}

std::vector<Attributes> getAttributes(json json_object, size_t idx)
{
    size_t attributes_size = json_object[idx]["attributes"].size();

    std::vector<Attributes> attributes;
    string eye_type;
    for (int i = attributes_size - 1; i >= 0; i--)
    {
        string attribute_type = getString(json_object[idx]["attributes"][i]["trait_type"].dump());
        string attribute_value = getString(json_object[idx]["attributes"][i]["value"].dump());
        if (attribute_type == "Eyes")
        {
            eye_type = attribute_value;
        }
        if (attribute_value == "Thick Glasses")
        {
            attribute_value = attribute_value + "/" + eye_type;
        }
        attributes.emplace_back(attribute_type, attribute_value);
    }

    return attributes;
}

string formatPrisionerId(string prisoner_id_str)
{
    int len = prisoner_id_str.length();
    for (int i = 0; i < 5 - len; i++)
    {
        prisoner_id_str.insert(0, "0");
    }
    return prisoner_id_str;
}

void chiguiresLoop(SkeletonData *skeletonData, Atlas *atlas)
{
    SP_UNUSED(atlas);

    GifManager gif_manager;
    std::vector<std::vector<Attributes>> all_metadata;
    std::ifstream json_file(metadata_info);
    json json_object;
    json_file >> json_object;
    sf::Clock deltaClock;
    double init_time = deltaClock.getElapsedTime().asSeconds();
    initial_iteration = max(0, initial_iteration);
    final_iteration = min(int(json_object.size()), final_iteration + 1);

    for (int iteration = initial_iteration; iteration < final_iteration; iteration++)
    {
        SkeletonDrawable drawable(skeletonData);
        drawable.timeScale = 1;
        drawable.setUsePremultipliedAlpha(false);
        Skeleton *skeleton = drawable.skeleton;
        Skin skin("chiguire");
        std::vector<Attributes> attributes;

        attributes = getAttributes(json_object, iteration);
        size_t type_id = attributes.size() - 2;
        size_t body_id = attributes.size() - 3;
        size_t face_id = attributes.size() - 4;
        size_t robot_body_id = attributes.size() - 4;

        if (attributes.size() == 1)
        {
            type_id = 0;
        }

        for (Attributes &attribute : attributes)
        {
            // Print current attributes if debug is enabled
            if (debug_attributes)
                std::cout << attribute.getFullName() << endl;

            // Add Skins
            if (attribute.value != "None")
            {
                skin.addSkin(skeletonData->findSkin(attribute.getFullName().c_str()));
            }
            // Add Ears when Natural Head is None
            else if (attribute.trait_type == "Head" && attributes[type_id].value == "Natural")
            {
                skin.addSkin(skeletonData->findSkin("Type/Natural Ears"));
            }
        }

        if (attributes[type_id].value == "Natural")
        {
            drawable.state->addAnimation(0, "Idle Natural", true, 0);
        }
        else if (attributes[type_id].value == "Slime")
        {
            drawable.state->addAnimation(0, "Idle Slime", false, 0);
        }
        else if (attributes[type_id].value == "Robot")
        {
            drawable.state->addAnimation(0, "Idle Robot", false, 0);
        }

        // Skin configuration
        skeleton->setSkin(&skin);
        skeleton->setSlotsToSetupPose();
        skeleton->setPosition(IMAGE_SIZE / 2, IMAGE_SIZE / 2);
        skeleton->updateWorldTransform();
        drawable.state->setListener(callback);

        // Render configuration
        sf::RenderTexture texture;
        texture.create(IMAGE_SIZE, IMAGE_SIZE);

        // Gif manager configuration
        int delay = 3;              // * 0.01 ms = 30ms
        float delta = 0.01 * delay; // ms
        int quality = 30;
        int size = IMAGE_SIZE;
        int num_frames = 101;
        bool gloabal_color = true;
        GifEncoder::PixelFormat Format = GifEncoder::PixelFormat::PIXEL_FORMAT_RGBA;
        gif_manager.configure(quality, delay, gloabal_color, Format, false);

        // Start gif creation
        if (gif_collection)
        {
            if (!merge_gifs)
            {
                std::string gif_name = output_path + std::to_string(iteration) + ".gif";
                gif_manager.startGif(gif_name, size, size, num_frames);
            }
            else if (!gif_started)
            {
                std::string gif_name = output_path + std::to_string(0) + ".gif";
                gif_manager.startGif(
                    gif_name, size, size, frames_per_gif * (final_iteration - initial_iteration));
                gif_started = true;
            }
        }

        animation_completed = false;
        sf::Image canvas_frame;
        bool firsttime = true;
        size_t count = 0;
        while (true)
        {
            // Update animation
            if (firsttime || animation_completed)
            {
                if (merge_gifs)
                {
                    drawable.update(delta + delta_offset);
                    animation_completed = false;
                }
                else
                {
                    drawable.update(0);
                }
                firsttime = false;
            }
            else
                drawable.update(delta);

            delta_offset += delta;
            if (delta_offset > 2.0)
            {
                delta_offset = 0;
                animation_completed = true;
            }

            texture.draw(drawable);
            count++;
            if (merge_gifs && (count > frames_per_gif))
            {
                break;
            }
            // If animation is finished, then save gif file
            if (animation_completed && gif_collection && !merge_gifs)
            {
                gif_manager.endGif();
                break;
            }
            // Otherwise, Save capture to gif object
            canvas_frame = texture.getTexture().copyToImage();
            canvas_frame.flipVertically();
            if (img_collection)
            {
                std::string img_filename;
                img_filename = "../results/img/" + std::to_string(iteration) + ".jpg";

                canvas_frame.saveToFile(img_filename);
                std::cout << "Finished IMG: " << img_filename << std::endl;
                break;
            }

            if (gif_collection)
                gif_manager.addFrame(canvas_frame);
        }
    }
    if (merge_gifs)
    {
        gif_manager.endGif();
    }
    double end_time = deltaClock.getElapsedTime().asSeconds();
    cout << "\nTOTAL Time: " << end_time - init_time << endl;
}

DebugExtension dbgExtension(SpineExtension::getInstance());

int main(int argc, char *argv[])
{
    // SpineExtension::setInstance(&dbgExtension);
    namespace po = boost::program_options;
    po::options_description description("Usage:");

    string path_scene, path_sensor_config, path_output;
    bool sneak_peeks = false;

    description.add_options()("help,h", "Display this help message")(
        "debug,d", po::bool_switch(&debug_attributes), "Create Image collection.")(
        "sneak_peeks,p", po::bool_switch(&sneak_peeks), "Create sneak_peeks collection.")(
        "init,i", po::value<int>(), "Initial nft")("end,e", po::value<int>(), "Final nft")(
        "single,s", po::value<int>(), "Single nft")("merge,m", po::value<int>(), "Merge gifs")(
        "format,f", po::value<std::string>()->default_value("0.3"), "Format (scale)")(
        "img", po::bool_switch(&img_collection), "Create Image collection.")(
        "output,o", po::value<std::string>()->default_value("gif"), "Output path");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(description).run(), vm);
    po::notify(vm);

    // Help and Version
    if (vm.count("help"))
    {
        cout << description;
        return 0;
    }
    else if (verify_collection)
    {
        cout << "Verify integrity.\n";
    }
    else if (img_collection)
    {
        cout << "Img Collection.\n";
        gif_collection = false;
    }
    else
    {
        cout << "Gif Collection (default).\n";
        gif_collection = true;
    }
    if (sneak_peeks)
    {
        metadata_info = "../metadata/sneak_peeks.json";
        cout << "Using custom metadata file: " << metadata_info << endl;
    }
    if (debug_attributes)
    {
        cout << "Debug active.\n";
    }
    if (vm.count("init"))
    {
        initial_iteration = vm["init"].as<int>() - 1;
        cout << "Initial iteration: " << initial_iteration + 1 << endl;
    }
    if (vm.count("end"))
    {
        final_iteration = vm["end"].as<int>() - 1;
        cout << "Final iteration: " << final_iteration + 1 << endl;
    }
    if (vm.count("single"))
    {
        initial_iteration = vm["single"].as<int>() - 1;
        final_iteration = vm["single"].as<int>() - 1;
        cout << "Single iteration: " << final_iteration + 1 << endl;
    }
    if (vm.count("merge"))
    {
        merge_gifs = true;
        frames_per_gif = vm["merge"].as<int>();
        cout << "Merge gifs, " << frames_per_gif << " frames per gif.";
    }
    else
    {
        merge_gifs = false;
    }
    if (vm.count("output"))
    {
        output_path = vm["output"].as<std::string>();
        cout << "Output path: " << output_path << endl;
        output_path = "../results/" + output_path + "/";
    }
    if (vm.count("format"))
    {
        scale_format = vm["format"].as<std::string>();
        SCALE = std::stof(scale_format);
        IMAGE_SIZE = 2000 * SCALE;
    }

    std::cout << "Start collection \n"
              << std::endl;

    string skel_name = data_path + "/" + project_name + ".skel";
    string atlas_name = data_path + "/" + project_name + scale_format + ".atlas";

    createGifCollection(chiguiresLoop, skel_name.c_str(), atlas_name.c_str(), SCALE);

    return 0;
}
