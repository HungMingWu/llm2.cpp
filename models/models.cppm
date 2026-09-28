module;
#include <cstdint>

export module models;
#define VOCAB_SIZE 262144
#define NUM_LAYERS 35

export
{
  struct Tensor {
      void *data;
      uint16_t *scales;
      int shape[4];
  };

  struct LayerWeights {
    Tensor input_layernorm;
    Tensor layer_scalar;
    Tensor pre_ffn_layernorm;
    Tensor post_attn_layernorm;
    Tensor post_ffn_layernorm;
    Tensor post_per_layer_input_norm;
    Tensor per_layer_input_gate;
    Tensor per_layer_projection;
    Tensor q_norm;
    Tensor k_norm;
    Tensor q_proj;
    Tensor k_proj;
    Tensor v_proj;
    Tensor o_proj;
    Tensor gate_proj;
    Tensor up_proj;
    Tensor down_proj;
    Tensor rope_cos;
    Tensor rope_sin;
  };

  struct ModelWeights {
    Tensor embed;
    Tensor embed_per_layer;
    LayerWeights layers[NUM_LAYERS];
    Tensor norm;
    Tensor per_layer_model_projection;
    Tensor per_layer_projection_norm;
    Tensor gelu_table;
  };

  struct VocabEntry {
    char token[94]; // The longest vocabulary piece is 93 bytes plus the null terminator.
    int id;
  };

  struct LookupEntry {
    char key[8]; // Holds one UTF-8 piece or a pair of 32-bit token IDs.
    int result, rank;
  };

  struct Tokenizer {
    int merge_count;
    int encode_vocab_count;
    int special_count;
    char decoded_tokens[VOCAB_SIZE][94];
    VocabEntry specials[256]; // Reserves space for the checkpoint's 24 special tokens.
    LookupEntry encode_vocab[32768]; // Reserves space for 19,249 directly encoded pieces.
    LookupEntry merges[514906]; // This checkpoint contains 514,906 merge rules.
  };

  struct Model {
    char magic[4];
    Tokenizer tokenizer;
    ModelWeights weights;
  };
}
