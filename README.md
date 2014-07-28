cosl
====

Cross platform shader language

Translates a language that is effectivly GLSL with a diffrent interface for textures, input/output and the like to actual GLSL or HLSL. This allows someone to write their shader once, then translate it to the platform that it needs. COSL is a bit dependent on libQEG, expecially the GLSL side, which names things in a way the libQEG expects. This could probabaly be changed in the future.
