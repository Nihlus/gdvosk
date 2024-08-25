// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT


#ifndef VOSKSPEAKERMODEL_H
#define VOSKSPEAKERMODEL_H

#include <godot_cpp/classes/resource.hpp>
#include <vosk/vosk_api.h>

namespace gdvosk
{
    /**
     * Represents a Vosk speaker model as a Godot resource.
     */
    class VoskSpeakerModel final : public godot::Resource
    {
        GDCLASS(VoskSpeakerModel, godot::Resource)

        friend class VoskRecognizer;

        /**
         * Holds the underlying pointer to the model.
         */
        VoskSpkModel* _model = nullptr;

    public:
        /**
         * Destroys an instance of the VoskSpeakerModel class.
         */
        ~VoskSpeakerModel() override;

        /**
         * Loads a model from the given path. Vosk only supports on-disk models as a tree of model files, and as such
         * this path must either be an absolute filesystem path or a user:// resource URI.
         * @param path The path to the model.
         * @return The result of the operation.
         */
        godot::Error load(const godot::String& path);

    protected:
        static void _bind_methods();

    private:
        /**
         * Gets the underlying pointer to the model.
         * @return The pointer.
         */
        [[nodiscard]] VoskSpkModel* get_ptr() const;
    };
}

#endif //VOSKSPEAKERMODEL_H
