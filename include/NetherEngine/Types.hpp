#pragma once

struct Uint2
{
    uint32_t x{};
    uint32_t y{};

    auto operator<=>(const Uint2& other) const = default;
};

struct Pipeline
{
    Comptr<ID3D12RootSignature> rootSignature{};
    Comptr<ID3D12PipelineState> pipelineState{};
};