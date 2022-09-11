//���� ��ü�� ������ ���� ��� ���۸� �����Ѵ�. 
cbuffer cbGameObjectInfo : register(b0)
{
	matrix worldMatrix : packoffset(c0);
};

//ī�޶��� ������ ���� ��� ���۸� �����Ѵ�. 
cbuffer cbCameraInfo : register(b1)
{
	matrix viewMatrix : packoffset(c0);
	matrix projMatrix : packoffset(c4);
};

//���� ���̴��� �Է��� ���� ����ü�� �����Ѵ�. 
struct VS_INPUT
{
	float4 position : POSITION;
	float4 color : COLOR;
	matrix worldMatrix : INSTANCE;
};

//���� ���̴��� ���(�ȼ� ���̴��� �Է�)�� ���� ����ü�� �����Ѵ�. 
struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

//���� ���̴��� �����Ѵ�. 
VS_OUTPUT VSMain(VS_INPUT input)
{
	VS_OUTPUT output;
	output.position = mul(input.position, input.worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projMatrix);
	output.color = input.color;
	return(output);
}

//�ȼ� ���̴��� �����Ѵ�. 
float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
	return(input.color);
}