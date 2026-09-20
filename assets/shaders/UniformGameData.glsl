layout(binding = 0, set = 0) uniform UniformGameData {
    mat4 projView;
    mat4 proj;
    mat4 view;

    vec3 cameraPos;
    float _pad0;

    vec3 cameraDir;
    float _pad1;

    vec3 ambientColor;
    float fogDensity;

    mat4 lightProjView;
    vec3 lightDir;
    float _pad2;
} uGameData;