#version 330 core

layout(location = 0) in vec4 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec3 Color;
layout(location = 3) in vec2 TexCoord;
layout(location = 4) in vec4 BoneWeights;
layout(location = 5) in uvec4 BoneIndices;

out vec4 position;
out vec3 normal;
out vec3 color;
out vec2 texCoord;
out vec4 shadow;

uniform mat4 mvp;
uniform mat3 itmv;
uniform mat4x3 bones[40]; // make sure this is more than the number of bones
uniform mat4 depth_bias_mvp;
uniform bool animated;

void main()
{
	if (animated)
	{
		position = vec4(BoneWeights.x * (bones[BoneIndices.x] * Position) +
						BoneWeights.y * (bones[BoneIndices.y] * Position) +
						BoneWeights.z * (bones[BoneIndices.z] * Position) +
						BoneWeights.w * (bones[BoneIndices.w] * Position), 1.0);
		normal = inverse(transpose(BoneWeights.x * mat3(bones[BoneIndices.x]) +
								   BoneWeights.y * mat3(bones[BoneIndices.y]) +
								   BoneWeights.z * mat3(bones[BoneIndices.z]) +
								   BoneWeights.w * mat3(bones[BoneIndices.w]))) * Normal;
	}
	else
	{
		position = Position;
		normal = itmv * Normal;
	}
	color = Color;
	texCoord = TexCoord;
	shadow = depth_bias_mvp * position;
	gl_Position = mvp * position;
}