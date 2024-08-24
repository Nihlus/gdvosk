// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT

#ifndef VOSKMODEL_H
#define VOSKMODEL_H

#include <godot_cpp/classes/resource.hpp>
#include <vosk/vosk_api.h>

namespace gdvosk
{
    class VoskRecognizer;

    /**
     * Represents a Vosk language model as a Godot resource.
     */
    class VoskModel final : public godot::Resource
    {
        GDCLASS(VoskModel, godot::Resource)

        friend class gdvosk::VoskRecognizer;

        /**
         * Holds the underlying pointer to the model.
         */
        ::VoskModel* _model = nullptr;

    public:
        /**
         * Destroys an instance of the VoskModel class.
         */
        ~VoskModel() override;

        /**
         * Searches the model for the given word. The model can only recognize words in its data set.
         * @param word The word to search for.
         * @return The symbol in the model for the word, or -1 if the word does not exist in the model.
         */
        [[nodiscard]] int find_word(const godot::String& word) const;

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
        [[nodiscard]] ::VoskModel* get_ptr() const;
    };
}

#endif //VOSKMODEL_H
