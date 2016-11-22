﻿#include "pch.h"
#include "Sample3DSceneRenderer.h"
#include "..\\Common\\DirectXHelper.h"
#include <fstream>
using namespace DX11UWA;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
Sample3DSceneRenderer::Sample3DSceneRenderer( const std::shared_ptr<DX::DeviceResources>& deviceResources ) :
	m_loadingComplete( false ),
	m_degreesPerSecond( 45 ),
	m_indexCount( 0 ),
	m_tracking( false ),
	m_deviceResources( deviceResources )
{
	memset( m_kbuttons, 0, sizeof( m_kbuttons ) );
	m_currMousePos = nullptr;
	m_prevMousePos = nullptr;
	memset( &m_camera, 0, sizeof( DirectX::XMFLOAT4X4 ) );

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void Sample3DSceneRenderer::CreateWindowSizeDependentResources( void )
{
	Windows::Foundation::Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * DirectX::XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if ( aspectRatio < 1.0f )
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	DirectX::XMMATRIX perspectiveMatrix = DirectX::XMMatrixPerspectiveFovLH( fovAngleY, aspectRatio, 0.01f, 100.0f );

	DirectX::XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	DirectX::XMMATRIX orientationMatrix = XMLoadFloat4x4( &orientation );

	XMStoreFloat4x4( &m_constantBufferData.projection, XMMatrixTranspose( perspectiveMatrix * orientationMatrix ) );

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const DirectX::XMVECTORF32 eye = { 0.0f, 0.7f, -1.5f, 0.0f };
	static const DirectX::XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const DirectX::XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4( &m_camera, XMMatrixInverse( nullptr, XMMatrixLookAtLH( eye, at, up ) ) );
	XMStoreFloat4x4( &m_constantBufferData.view, XMMatrixTranspose( XMMatrixLookAtLH( eye, at, up ) ) );
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update( DX::StepTimer const& timer )
{
	if ( !m_tracking )
	{
		// Convert degrees to radians, then convert seconds to rotation angle
		float radiansPerSecond = DirectX::XMConvertToRadians( m_degreesPerSecond );
		double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		float radians = static_cast< float >( fmod( totalRotation, DirectX::XM_2PI ) );

		Rotate( radians );
	}


	// Update or move camera here
	UpdateCamera( timer, 1.0f, 0.75f );

}

// Rotate the 3D cube model a set amount of radians.
void Sample3DSceneRenderer::Rotate( float radians )
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4( &m_constantBufferData.model, XMMatrixTranspose( DirectX::XMMatrixRotationY( radians ) ) );
}

void Sample3DSceneRenderer::UpdateCamera( DX::StepTimer const& timer, float const moveSpd, float const rotSpd )
{
	const float delta_time = ( float )timer.GetElapsedSeconds();

	if ( m_kbuttons[ 'W' ] )
	{
		DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation( 0.0f, 0.0f, moveSpd * delta_time );
		DirectX::XMMATRIX temp_camera = XMLoadFloat4x4( &m_camera );
		DirectX::XMMATRIX result = XMMatrixMultiply( translation, temp_camera );
		XMStoreFloat4x4( &m_camera, result );
	}
	if ( m_kbuttons[ 'S' ] )
	{
		DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation( 0.0f, 0.0f, -moveSpd * delta_time );
		DirectX::XMMATRIX temp_camera = XMLoadFloat4x4( &m_camera );
		DirectX::XMMATRIX result = XMMatrixMultiply( translation, temp_camera );
		XMStoreFloat4x4( &m_camera, result );
	}
	if ( m_kbuttons[ 'A' ] )
	{
		DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation( -moveSpd * delta_time, 0.0f, 0.0f );
		DirectX::XMMATRIX temp_camera = XMLoadFloat4x4( &m_camera );
		DirectX::XMMATRIX result = XMMatrixMultiply( translation, temp_camera );
		XMStoreFloat4x4( &m_camera, result );
	}
	if ( m_kbuttons[ 'D' ] )
	{
		DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation( moveSpd * delta_time, 0.0f, 0.0f );
		DirectX::XMMATRIX temp_camera = XMLoadFloat4x4( &m_camera );
		DirectX::XMMATRIX result = XMMatrixMultiply( translation, temp_camera );
		XMStoreFloat4x4( &m_camera, result );
	}
	if ( m_kbuttons[ 'X' ] )
	{
		DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation( 0.0f, -moveSpd * delta_time, 0.0f );
		DirectX::XMMATRIX temp_camera = XMLoadFloat4x4( &m_camera );
		DirectX::XMMATRIX result = XMMatrixMultiply( translation, temp_camera );
		XMStoreFloat4x4( &m_camera, result );
	}
	if ( m_kbuttons[ VK_SPACE ] )
	{
		DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation( 0.0f, moveSpd * delta_time, 0.0f );
		DirectX::XMMATRIX temp_camera = XMLoadFloat4x4( &m_camera );
		DirectX::XMMATRIX result = XMMatrixMultiply( translation, temp_camera );
		XMStoreFloat4x4( &m_camera, result );
	}

	if ( m_currMousePos )
	{
		if ( m_currMousePos->Properties->IsRightButtonPressed && m_prevMousePos )
		{
			float dx = m_currMousePos->Position.X - m_prevMousePos->Position.X;
			float dy = m_currMousePos->Position.Y - m_prevMousePos->Position.Y;

			DirectX::XMFLOAT4 pos( m_camera._41, m_camera._42, m_camera._43, m_camera._44 );

			m_camera._41 = 0.0f;
			m_camera._42 = 0.0f;
			m_camera._43 = 0.0f;

			DirectX::XMMATRIX rotX = DirectX::XMMatrixRotationX( dy * rotSpd * delta_time );
			DirectX::XMMATRIX rotY = DirectX::XMMatrixRotationY( dx * rotSpd * delta_time );

			DirectX::XMMATRIX temp_camera = XMLoadFloat4x4( &m_camera );
			temp_camera = XMMatrixMultiply( rotX, temp_camera );
			temp_camera = XMMatrixMultiply( temp_camera, rotY );

			XMStoreFloat4x4( &m_camera, temp_camera );

			m_camera._41 = pos.x;
			m_camera._42 = pos.y;
			m_camera._43 = pos.z;
		}
		m_prevMousePos = m_currMousePos;
	}


}

void Sample3DSceneRenderer::SetKeyboardButtons( const char* list )
{
	memcpy_s( m_kbuttons, sizeof( m_kbuttons ), list, sizeof( m_kbuttons ) );
}

void Sample3DSceneRenderer::SetMousePosition( const Windows::UI::Input::PointerPoint^ pos )
{
	m_currMousePos = const_cast< Windows::UI::Input::PointerPoint^ >( pos );
}

void Sample3DSceneRenderer::SetInputDeviceData( const char* kb, const Windows::UI::Input::PointerPoint^ pos )
{
	SetKeyboardButtons( kb );
	SetMousePosition( pos );
}

void DX11UWA::Sample3DSceneRenderer::StartTracking( void )
{
	m_tracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void Sample3DSceneRenderer::TrackingUpdate( float positionX )
{
	if ( m_tracking )
	{
		float radians = DirectX::XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate( radians );
	}
}

void Sample3DSceneRenderer::StopTracking( void )
{
	m_tracking = false;
}

// Renders one frame using the vertex and pixel shaders.
void Sample3DSceneRenderer::Render( void )
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if ( !m_loadingComplete )
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	XMStoreFloat4x4( &m_constantBufferData.view, XMMatrixTranspose( XMMatrixInverse( nullptr, XMLoadFloat4x4( &m_camera ) ) ) );


	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1( m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0 );
	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof( VertexPositionColor );
	UINT offset = 0;
	context->IASetVertexBuffers( 0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset );
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer( m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0 );
	context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	context->IASetInputLayout( m_inputLayout.Get() );
	// Attach our vertex shader.
	context->VSSetShader( m_vertexShader.Get(), nullptr, 0 );
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1( 0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr );
	// Attach our pixel shader.
	context->PSSetShader( m_pixelShader.Get(), nullptr, 0 );
	// Draw the objects.
	context->DrawIndexed( m_indexCount, 0, 0 );
}

void Sample3DSceneRenderer::ObjMesh_CountLines(
	const char* const filepath,
	unsigned int& positions,
	unsigned int& uvs,
	unsigned int& normals,
	unsigned int& faces
)
{
	std::ifstream file;
	file.open( filepath, std::ios_base::in );
	if ( file.is_open() )
	{

		file.close();
	}
}

void Sample3DSceneRenderer::ObjMesh_LoadMesh(
	const char* const filepath,
	VertexPositionColor*& outVertices,
	unsigned short*& outIndices,
	unsigned int& outNumVertices,
	unsigned int& outNumIndices )
{
	unsigned int numPositions = 0u, numUVs = 0u, numNormals = 0u, numFaces = 0u;
	ObjMesh_CountLines( filepath, numPositions, numUVs, numNormals, numFaces );
	std::ifstream file;
	file.open( filepath, std::ios_base::in );
	if ( file.is_open() )
	{

		file.close();
	}
#pragma region temporary
	static const VertexPositionColor tempVertices[ ] =
	{
		{ DirectX::XMFLOAT3( -0.5f, -0.5f, -0.5f ), DirectX::XMFLOAT3( 0.0f, 0.0f, 0.0f ) },
		{ DirectX::XMFLOAT3( -0.5f, -0.5f, 0.5f ), DirectX::XMFLOAT3( 0.0f, 0.0f, 1.0f ) },
		{ DirectX::XMFLOAT3( -0.5f, 0.5f, -0.5f ), DirectX::XMFLOAT3( 0.0f, 1.0f, 0.0f ) },
		{ DirectX::XMFLOAT3( -0.5f, 0.5f, 0.5f ), DirectX::XMFLOAT3( 0.0f, 1.0f, 1.0f ) },
		{ DirectX::XMFLOAT3( 0.5f, -0.5f, -0.5f ), DirectX::XMFLOAT3( 1.0f, 0.0f, 0.0f ) },
		{ DirectX::XMFLOAT3( 0.5f, -0.5f, 0.5f ), DirectX::XMFLOAT3( 1.0f, 0.0f, 1.0f ) },
		{ DirectX::XMFLOAT3( 0.5f, 0.5f, -0.5f ), DirectX::XMFLOAT3( 1.0f, 1.0f, 0.0f ) },
		{ DirectX::XMFLOAT3( 0.5f, 0.5f, 0.5f ), DirectX::XMFLOAT3( 1.0f, 1.0f, 1.0f ) },
	};
	static const unsigned short tempIndices[ ] =
	{
		0ui16, 1ui16, 2ui16,
		1ui16, 3ui16, 2ui16,

		4ui16, 6ui16, 5ui16,
		5ui16, 6ui16, 7ui16,

		0ui16, 5ui16, 1ui16,
		0ui16, 4ui16, 5ui16,

		2ui16, 7ui16, 6ui16,
		2ui16, 3ui16, 7ui16,

		0ui16, 6ui16, 4ui16,
		0ui16, 2ui16, 6ui16,

		1ui16, 7ui16, 3ui16,
		1ui16, 5ui16, 7ui16
	};
	outNumVertices = 8u;
	outNumIndices = 36u;
	outVertices = new VertexPositionColor[ outNumVertices ];
	outIndices = new unsigned short[ outNumIndices ];
	memcpy_s( outVertices, sizeof( VertexPositionColor ) * outNumVertices, tempVertices, sizeof( tempVertices ) );
	memcpy_s( outIndices, sizeof( unsigned short ) * outNumIndices, tempIndices, sizeof( tempIndices ) );
#pragma endregion
}

void Sample3DSceneRenderer::ObjMesh_Unload(
	VertexPositionColor*& vertices,
	unsigned short*& indices )
{
	delete[ ] vertices;
	delete[ ] indices;
	vertices = nullptr;
	indices = nullptr;
}

void Sample3DSceneRenderer::CreateDeviceDependentResources( void )
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync( L"SampleVertexShader.cso" );
	auto loadPSTask = DX::ReadDataAsync( L"SamplePixelShader.cso" );
	auto createVSTask = loadVSTask.then( [ this ]( const std::vector<byte>& fileData )
	{
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateVertexShader( &fileData[ 0 ], fileData.size(), nullptr, &m_vertexShader ) );

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[ ] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateInputLayout( vertexDesc, ARRAYSIZE( vertexDesc ), &fileData[ 0 ], fileData.size(), &m_inputLayout ) );
	} );
	auto createPSTask = loadPSTask.then( [ this ]( const std::vector<byte>& fileData )
	{
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreatePixelShader( &fileData[ 0 ], fileData.size(), nullptr, &m_pixelShader ) );

		CD3D11_BUFFER_DESC constantBufferDesc( sizeof( ModelViewProjectionConstantBuffer ), D3D11_BIND_CONSTANT_BUFFER );
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateBuffer( &constantBufferDesc, nullptr, &m_constantBuffer ) );
	} );
	auto createMeshTask = ( createVSTask && createPSTask ).then( [ this ]()
	{
		VertexPositionColor* vertices = nullptr;
		unsigned short* indices = nullptr;
		unsigned int numVertices = 0u, numIndices = 0u;

		ObjMesh_LoadMesh( "Assets\\Mesh.mobj", vertices, indices, numVertices, numIndices );

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		ZEROSTRUCT( vertexBufferData );
		vertexBufferData.pSysMem = vertices;
		CD3D11_BUFFER_DESC vertexBufferDesc( sizeof( VertexPositionColor ) * numVertices, D3D11_BIND_VERTEX_BUFFER );
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &m_vertexBuffer ) );

		m_indexCount = numIndices;

		D3D11_SUBRESOURCE_DATA indexBufferData;
		ZEROSTRUCT( indexBufferData );
		indexBufferData.pSysMem = indices;
		CD3D11_BUFFER_DESC indexBufferDesc( sizeof( unsigned short ) * numIndices, D3D11_BIND_INDEX_BUFFER );
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateBuffer( &indexBufferDesc, &indexBufferData, &m_indexBuffer ) );

		ObjMesh_Unload( vertices, indices );
	} );
	createMeshTask.then( [ this ]() { m_loadingComplete = true; } );
}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources( void )
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}