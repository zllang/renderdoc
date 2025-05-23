/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2020-2025 Baldur Karlsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#pragma once

#include "dxil_bytecode.h"

namespace DXIL
{
enum DW_LANG
{
  DW_LANG_Unknown = 0,
  DW_LANG_C89 = 0x0001,
  DW_LANG_C = 0x0002,
  DW_LANG_Ada83 = 0x0003,
  DW_LANG_C_plus_plus = 0x0004,
  DW_LANG_Cobol74 = 0x0005,
  DW_LANG_Cobol85 = 0x0006,
  DW_LANG_Fortran77 = 0x0007,
  DW_LANG_Fortran90 = 0x0008,
  DW_LANG_Pascal83 = 0x0009,
  DW_LANG_Modula2 = 0x000a,
  DW_LANG_Java = 0x000b,
  DW_LANG_C99 = 0x000c,
  DW_LANG_Ada95 = 0x000d,
  DW_LANG_Fortran95 = 0x000e,
  DW_LANG_PLI = 0x000f,
  DW_LANG_ObjC = 0x0010,
  DW_LANG_ObjC_plus_plus = 0x0011,
  DW_LANG_UPC = 0x0012,
  DW_LANG_D = 0x0013,
  DW_LANG_Python = 0x0014,
  DW_LANG_OpenCL = 0x0015,
  DW_LANG_Go = 0x0016,
  DW_LANG_Modula3 = 0x0017,
  DW_LANG_Haskell = 0x0018,
  DW_LANG_C_plus_plus_03 = 0x0019,
  DW_LANG_C_plus_plus_11 = 0x001a,
  DW_LANG_OCaml = 0x001b,
  DW_LANG_Rust = 0x001c,
  DW_LANG_C11 = 0x001d,
  DW_LANG_Swift = 0x001e,
  DW_LANG_Julia = 0x001f,
  DW_LANG_Dylan = 0x0020,
  DW_LANG_C_plus_plus_14 = 0x0021,
  DW_LANG_Fortran03 = 0x0022,
  DW_LANG_Fortran08 = 0x0023,
  DW_LANG_Mips_Assembler = 0x8001,
};

enum DW_TAG
{
  DW_TAG_array_type = 0x0001,
  DW_TAG_class_type = 0x0002,
  DW_TAG_entry_point = 0x0003,
  DW_TAG_enumeration_type = 0x0004,
  DW_TAG_formal_parameter = 0x0005,
  DW_TAG_imported_declaration = 0x0008,
  DW_TAG_label = 0x000a,
  DW_TAG_lexical_block = 0x000b,
  DW_TAG_member = 0x000d,
  DW_TAG_pointer_type = 0x000f,
  DW_TAG_reference_type = 0x0010,
  DW_TAG_compile_unit = 0x0011,
  DW_TAG_string_type = 0x0012,
  DW_TAG_structure_type = 0x0013,
  DW_TAG_subroutine_type = 0x0015,
  DW_TAG_typedef = 0x0016,
  DW_TAG_union_type = 0x0017,
  DW_TAG_unspecified_parameters = 0x0018,
  DW_TAG_variant = 0x0019,
  DW_TAG_common_block = 0x001a,
  DW_TAG_common_inclusion = 0x001b,
  DW_TAG_inheritance = 0x001c,
  DW_TAG_inlined_subroutine = 0x001d,
  DW_TAG_module = 0x001e,
  DW_TAG_ptr_to_member_type = 0x001f,
  DW_TAG_set_type = 0x0020,
  DW_TAG_subrange_type = 0x0021,
  DW_TAG_with_stmt = 0x0022,
  DW_TAG_access_declaration = 0x0023,
  DW_TAG_base_type = 0x0024,
  DW_TAG_catch_block = 0x0025,
  DW_TAG_const_type = 0x0026,
  DW_TAG_constant = 0x0027,
  DW_TAG_enumerator = 0x0028,
  DW_TAG_file_type = 0x0029,
  DW_TAG_friend = 0x002a,
  DW_TAG_namelist = 0x002b,
  DW_TAG_namelist_item = 0x002c,
  DW_TAG_packed_type = 0x002d,
  DW_TAG_subprogram = 0x002e,
  DW_TAG_template_type_parameter = 0x002f,
  DW_TAG_template_value_parameter = 0x0030,
  DW_TAG_thrown_type = 0x0031,
  DW_TAG_try_block = 0x0032,
  DW_TAG_variant_part = 0x0033,
  DW_TAG_variable = 0x0034,
  DW_TAG_volatile_type = 0x0035,
  DW_TAG_dwarf_procedure = 0x0036,
  DW_TAG_restrict_type = 0x0037,
  DW_TAG_interface_type = 0x0038,
  DW_TAG_namespace = 0x0039,
  DW_TAG_imported_module = 0x003a,
  DW_TAG_unspecified_type = 0x003b,
  DW_TAG_partial_unit = 0x003c,
  DW_TAG_imported_unit = 0x003d,
  DW_TAG_condition = 0x003f,
  DW_TAG_shared_type = 0x0040,
  DW_TAG_type_unit = 0x0041,
  DW_TAG_rvalue_reference_type = 0x0042,
  DW_TAG_template_alias = 0x0043,
  DW_TAG_auto_variable = 0x0100,
  DW_TAG_arg_variable = 0x0101,
  DW_TAG_coarray_type = 0x0044,
  DW_TAG_generic_subrange = 0x0045,
  DW_TAG_dynamic_type = 0x0046,
  DW_TAG_MIPS_loop = 0x4081,
  DW_TAG_format_label = 0x4101,
  DW_TAG_function_template = 0x4102,
  DW_TAG_class_template = 0x4103,
  DW_TAG_GNU_template_template_param = 0x4106,
  DW_TAG_GNU_template_parameter_pack = 0x4107,
  DW_TAG_GNU_formal_parameter_pack = 0x4108,
  DW_TAG_APPLE_property = 0x4200,
};

enum DW_ENCODING
{
  DW_ATE_address = 0x01,
  DW_ATE_boolean = 0x02,
  DW_ATE_complex_float = 0x03,
  DW_ATE_float = 0x04,
  DW_ATE_signed = 0x05,
  DW_ATE_signed_char = 0x06,
  DW_ATE_unsigned = 0x07,
  DW_ATE_unsigned_char = 0x08,
  DW_ATE_imaginary_float = 0x09,
  DW_ATE_packed_decimal = 0x0a,
  DW_ATE_numeric_string = 0x0b,
  DW_ATE_edited = 0x0c,
  DW_ATE_signed_fixed = 0x0d,
  DW_ATE_unsigned_fixed = 0x0e,
  DW_ATE_decimal_float = 0x0f,
  DW_ATE_UTF = 0x10,
};

enum DW_VIRTUALITY
{
  DW_VIRTUALITY_none = 0x00,
  DW_VIRTUALITY_virtual = 0x01,
  DW_VIRTUALITY_pure_virtual = 0x02,
};

enum DW_OP
{
  DW_OP_none = 0x0,
  DW_OP_addr = 0x03,
  DW_OP_deref = 0x06,
  DW_OP_const1u = 0x08,
  DW_OP_const1s = 0x09,
  DW_OP_const2u = 0x0a,
  DW_OP_const2s = 0x0b,
  DW_OP_const4u = 0x0c,
  DW_OP_const4s = 0x0d,
  DW_OP_const8u = 0x0e,
  DW_OP_const8s = 0x0f,
  DW_OP_constu = 0x10,
  DW_OP_consts = 0x11,
  DW_OP_dup = 0x12,
  DW_OP_drop = 0x13,
  DW_OP_over = 0x14,
  DW_OP_pick = 0x15,
  DW_OP_swap = 0x16,
  DW_OP_rot = 0x17,
  DW_OP_xderef = 0x18,
  DW_OP_abs = 0x19,
  DW_OP_and = 0x1a,
  DW_OP_div = 0x1b,
  DW_OP_minus = 0x1c,
  DW_OP_mod = 0x1d,
  DW_OP_mul = 0x1e,
  DW_OP_neg = 0x1f,
  DW_OP_not = 0x20,
  DW_OP_or = 0x21,
  DW_OP_plus = 0x22,
  DW_OP_plus_uconst = 0x23,
  DW_OP_shl = 0x24,
  DW_OP_shr = 0x25,
  DW_OP_shra = 0x26,
  DW_OP_xor = 0x27,
  DW_OP_skip = 0x2f,
  DW_OP_bra = 0x28,
  DW_OP_eq = 0x29,
  DW_OP_ge = 0x2a,
  DW_OP_gt = 0x2b,
  DW_OP_le = 0x2c,
  DW_OP_lt = 0x2d,
  DW_OP_ne = 0x2e,
  DW_OP_lit0 = 0x30,
  DW_OP_lit1 = 0x31,
  DW_OP_lit2 = 0x32,
  DW_OP_lit3 = 0x33,
  DW_OP_lit4 = 0x34,
  DW_OP_lit5 = 0x35,
  DW_OP_lit6 = 0x36,
  DW_OP_lit7 = 0x37,
  DW_OP_lit8 = 0x38,
  DW_OP_lit9 = 0x39,
  DW_OP_lit10 = 0x3a,
  DW_OP_lit11 = 0x3b,
  DW_OP_lit12 = 0x3c,
  DW_OP_lit13 = 0x3d,
  DW_OP_lit14 = 0x3e,
  DW_OP_lit15 = 0x3f,
  DW_OP_lit16 = 0x40,
  DW_OP_lit17 = 0x41,
  DW_OP_lit18 = 0x42,
  DW_OP_lit19 = 0x43,
  DW_OP_lit20 = 0x44,
  DW_OP_lit21 = 0x45,
  DW_OP_lit22 = 0x46,
  DW_OP_lit23 = 0x47,
  DW_OP_lit24 = 0x48,
  DW_OP_lit25 = 0x49,
  DW_OP_lit26 = 0x4a,
  DW_OP_lit27 = 0x4b,
  DW_OP_lit28 = 0x4c,
  DW_OP_lit29 = 0x4d,
  DW_OP_lit30 = 0x4e,
  DW_OP_lit31 = 0x4f,
  DW_OP_reg0 = 0x50,
  DW_OP_reg1 = 0x51,
  DW_OP_reg2 = 0x52,
  DW_OP_reg3 = 0x53,
  DW_OP_reg4 = 0x54,
  DW_OP_reg5 = 0x55,
  DW_OP_reg6 = 0x56,
  DW_OP_reg7 = 0x57,
  DW_OP_reg8 = 0x58,
  DW_OP_reg9 = 0x59,
  DW_OP_reg10 = 0x5a,
  DW_OP_reg11 = 0x5b,
  DW_OP_reg12 = 0x5c,
  DW_OP_reg13 = 0x5d,
  DW_OP_reg14 = 0x5e,
  DW_OP_reg15 = 0x5f,
  DW_OP_reg16 = 0x60,
  DW_OP_reg17 = 0x61,
  DW_OP_reg18 = 0x62,
  DW_OP_reg19 = 0x63,
  DW_OP_reg20 = 0x64,
  DW_OP_reg21 = 0x65,
  DW_OP_reg22 = 0x66,
  DW_OP_reg23 = 0x67,
  DW_OP_reg24 = 0x68,
  DW_OP_reg25 = 0x69,
  DW_OP_reg26 = 0x6a,
  DW_OP_reg27 = 0x6b,
  DW_OP_reg28 = 0x6c,
  DW_OP_reg29 = 0x6d,
  DW_OP_reg30 = 0x6e,
  DW_OP_reg31 = 0x6f,
  DW_OP_breg0 = 0x70,
  DW_OP_breg1 = 0x71,
  DW_OP_breg2 = 0x72,
  DW_OP_breg3 = 0x73,
  DW_OP_breg4 = 0x74,
  DW_OP_breg5 = 0x75,
  DW_OP_breg6 = 0x76,
  DW_OP_breg7 = 0x77,
  DW_OP_breg8 = 0x78,
  DW_OP_breg9 = 0x79,
  DW_OP_breg10 = 0x7a,
  DW_OP_breg11 = 0x7b,
  DW_OP_breg12 = 0x7c,
  DW_OP_breg13 = 0x7d,
  DW_OP_breg14 = 0x7e,
  DW_OP_breg15 = 0x7f,
  DW_OP_breg16 = 0x80,
  DW_OP_breg17 = 0x81,
  DW_OP_breg18 = 0x82,
  DW_OP_breg19 = 0x83,
  DW_OP_breg20 = 0x84,
  DW_OP_breg21 = 0x85,
  DW_OP_breg22 = 0x86,
  DW_OP_breg23 = 0x87,
  DW_OP_breg24 = 0x88,
  DW_OP_breg25 = 0x89,
  DW_OP_breg26 = 0x8a,
  DW_OP_breg27 = 0x8b,
  DW_OP_breg28 = 0x8c,
  DW_OP_breg29 = 0x8d,
  DW_OP_breg30 = 0x8e,
  DW_OP_breg31 = 0x8f,
  DW_OP_regx = 0x90,
  DW_OP_fbreg = 0x91,
  DW_OP_bregx = 0x92,
  DW_OP_piece = 0x93,
  DW_OP_deref_size = 0x94,
  DW_OP_xderef_size = 0x95,
  DW_OP_nop = 0x96,
  DW_OP_push_object_address = 0x97,
  DW_OP_call2 = 0x98,
  DW_OP_call4 = 0x99,
  DW_OP_call_ref = 0x9a,
  DW_OP_form_tls_address = 0x9b,
  DW_OP_call_frame_cfa = 0x9c,
  DW_OP_bit_piece = 0x9d,
  DW_OP_implicit_value = 0x9e,
  DW_OP_stack_value = 0x9f,
  DW_OP_GNU_push_tls_address = 0xe0,
  DW_OP_GNU_addr_index = 0xfb,
  DW_OP_GNU_const_index = 0xfc,
};

enum DIFlags
{
  DIFlagNone = 0,
  DIFlagPrivate = 1,
  DIFlagProtected = 2,
  DIFlagPublic = 3,
  DIFlagFwdDecl = (1 << 2),
  DIFlagAppleBlock = (1 << 3),
  DIFlagBlockByrefStruct = (1 << 4),
  DIFlagVirtual = (1 << 5),
  DIFlagArtificial = (1 << 6),
  DIFlagExplicit = (1 << 7),
  DIFlagPrototyped = (1 << 8),
  DIFlagObjcClassComplete = (1 << 9),
  DIFlagObjectPointer = (1 << 10),
  DIFlagVector = (1 << 11),
  DIFlagStaticMember = (1 << 12),
  DIFlagLValueReference = (1 << 13),
  DIFlagRValueReference = (1 << 14),
};

struct DIFile : public DIBase
{
  static const DIBase::Type DIType = DIBase::File;
  DIFile(const Metadata *file, const Metadata *dir) : DIBase(DIType), file(file), dir(dir) {}
  const Metadata *file;
  const Metadata *dir;

  virtual rdcstr toString(bool dxcStyleFormatting) const;
};

struct DICompileUnit : public DIBase
{
  static const DIBase::Type DIType = DIBase::CompileUnit;
  DICompileUnit(DW_LANG lang, const Metadata *file, const rdcstr *producer, bool isOptimized,
                const rdcstr *flags, uint64_t runtimeVersion, const rdcstr *splitDebugFilename,
                uint64_t emissionKind, const Metadata *enums, const Metadata *retainedTypes,
                const Metadata *subprograms, const Metadata *globals, const Metadata *imports)
      : DIBase(DIType),
        lang(lang),
        file(file),
        producer(producer),
        isOptimized(isOptimized),
        flags(flags),
        runtimeVersion(runtimeVersion),
        splitDebugFilename(splitDebugFilename),
        emissionKind(emissionKind),
        enums(enums),
        retainedTypes(retainedTypes),
        subprograms(subprograms),
        globals(globals),
        imports(imports)
  {
  }

  DW_LANG lang;
  const Metadata *file;
  const rdcstr *producer;
  bool isOptimized;
  const rdcstr *flags;
  uint64_t runtimeVersion;
  const rdcstr *splitDebugFilename;
  uint64_t emissionKind;
  const Metadata *enums;
  const Metadata *retainedTypes;
  const Metadata *subprograms;
  const Metadata *globals;
  const Metadata *imports;

  virtual rdcstr toString(bool dxcStyleFormatting) const;
};

struct DIBasicType : public DIBase
{
  static const DIBase::Type DIType = DIBase::BasicType;
  DIBasicType(DW_TAG tag, const rdcstr *name, uint64_t sizeInBits, uint64_t alignInBits,
              DW_ENCODING encoding)
      : DIBase(DIType),
        tag(tag),
        name(name),
        sizeInBits(sizeInBits),
        alignInBits(alignInBits),
        encoding(encoding)
  {
  }

  DW_TAG tag;
  const rdcstr *name;
  uint64_t sizeInBits;
  uint64_t alignInBits;
  DW_ENCODING encoding;

  virtual rdcstr toString(bool dxcStyleFormatting) const;
};

struct DIDerivedType : public DIBase
{
  static const DIBase::Type DIType = DIBase::DerivedType;
  DIDerivedType(DW_TAG tag, const rdcstr *name, const Metadata *file, uint64_t line,
                const Metadata *scope, const Metadata *base, uint64_t sizeInBits,
                uint64_t alignInBits, uint64_t offsetInBits, DIFlags flags, const Metadata *extra)
      : DIBase(DIType),
        tag(tag),
        name(name),
        file(file),
        line(line),
        scope(scope),
        base(base),
        sizeInBits(sizeInBits),
        alignInBits(alignInBits),
        offsetInBits(offsetInBits),
        flags(flags),
        extra(extra)
  {
  }

  DW_TAG tag;
  const rdcstr *name;
  const Metadata *file;
  uint64_t line;
  const Metadata *scope;
  const Metadata *base;
  uint64_t sizeInBits;
  uint64_t alignInBits;
  uint64_t offsetInBits;
  DIFlags flags;
  const Metadata *extra;

  virtual rdcstr toString(bool dxcStyleFormatting) const;
};

struct DICompositeType : public DIBase
{
  static const DIBase::Type DIType = DIBase::CompositeType;
  DICompositeType(DW_TAG tag, const rdcstr *name, const Metadata *file, uint64_t line,
                  const Metadata *scope, const Metadata *base, uint64_t sizeInBits,
                  uint64_t alignInBits, uint64_t offsetInBits, DIFlags flags,
                  const Metadata *elements, const Metadata *templateParams)
      : DIBase(DIType),
        tag(tag),
        name(name),
        file(file),
        line(line),
        scope(scope),
        base(base),
        sizeInBits(sizeInBits),
        alignInBits(alignInBits),
        offsetInBits(offsetInBits),
        flags(flags),
        elements(elements),
        templateParams(templateParams)
  {
  }

  DW_TAG tag;
  const rdcstr *name;
  const Metadata *file;
  uint64_t line;
  const Metadata *scope;
  const Metadata *base;
  uint64_t sizeInBits;
  uint64_t alignInBits;
  uint64_t offsetInBits;
  DIFlags flags;
  const Metadata *elements;
  const Metadata *templateParams;

  virtual rdcstr toString(bool dxcStyleFormatting) const;
};

struct DIEnum : public DIBase
{
  static const DIBase::Type DIType = DIBase::Enum;
  DIEnum(int64_t value, const rdcstr *name) : DIBase(DIType), value(value), name(name) {}
  int64_t value;
  const rdcstr *name;

  virtual rdcstr toString(bool dxcStyleFormatting) const;
};

struct DITemplateTypeParameter : public DIBase
{
  static const DIBase::Type DIType = DIBase::TemplateTypeParameter;
  DITemplateTypeParameter(const rdcstr *name, const Metadata *type)
      : DIBase(DIType), name(name), type(type)
  {
  }
  const rdcstr *name;
  const Metadata *type;

  virtual rdcstr toString(bool dxcStyleFormatting) const;
};

struct DITemplateValueParameter : public DIBase
{
  static const DIBase::Type DIType = DIBase::TemplateValueParameter;
  DITemplateValueParameter(DW_TAG tag, const rdcstr *name, const Metadata *type, const Metadata *value)
      : DIBase(DIType), tag(tag), name(name), type(type), value(value)
  {
  }

  DW_TAG tag;
  const rdcstr *name;
  const Metadata *type;
  const Metadata *value;

  virtual rdcstr toString(bool dxcStyleFormatting) const;
};

struct DISubprogram : public DIBase
{
  static const DIBase::Type DIType = DIBase::Subprogram;
  DISubprogram(const Metadata *scope, const rdcstr *name, const rdcstr *linkageName,
               const Metadata *file, uint64_t line, const Metadata *type, bool isLocal,
               bool isDefinition, uint64_t scopeLine, const Metadata *containingType,
               DW_VIRTUALITY virtuality, uint64_t virtualIndex, DIFlags flags, bool isOptimized,
               Metadata *function, const Metadata *templateParams, const Metadata *declaration,
               const Metadata *variables)
      : DIBase(DIType),
        scope(scope),
        name(name),
        linkageName(linkageName),
        file(file),
        line(line),
        type(type),
        isLocal(isLocal),
        isDefinition(isDefinition),
        scopeLine(scopeLine),
        containingType(containingType),
        virtuality(virtuality),
        virtualIndex(virtualIndex),
        flags(flags),
        isOptimized(isOptimized),
        function(function),
        templateParams(templateParams),
        declaration(declaration),
        variables(variables)
  {
  }

  const Metadata *scope;
  const rdcstr *name;
  const rdcstr *linkageName;
  const Metadata *file;
  uint64_t line;
  const Metadata *type;
  bool isLocal;
  bool isDefinition;
  uint64_t scopeLine;
  const Metadata *containingType;
  DW_VIRTUALITY virtuality;
  uint64_t virtualIndex;
  DIFlags flags;
  bool isOptimized;
  Metadata *function;
  const Metadata *templateParams;
  const Metadata *declaration;
  const Metadata *variables;

  virtual void setID(uint32_t ID)
  {
    if(function && function->id == ~0U)
      function->id = ID;
  }

  virtual rdcstr toString(bool dxcStyleFormatting) const;
};

struct DISubroutineType : public DIBase
{
  static const DIBase::Type DIType = DIBase::SubroutineType;
  DISubroutineType(const Metadata *types) : DIBase(DIType), types(types) {}
  const Metadata *types;

  virtual rdcstr toString(bool dxcStyleFormatting) const;
};

struct DIGlobalVariable : public DIBase
{
  static const DIBase::Type DIType = DIBase::GlobalVariable;
  DIGlobalVariable(const Metadata *scope, const rdcstr *name, const rdcstr *linkageName,
                   const Metadata *file, uint64_t line, const Metadata *type, bool isLocal,
                   bool isDefinition, const Metadata *variable, const Metadata *declaration)
      : DIBase(DIType),
        scope(scope),
        name(name),
        linkageName(linkageName),
        file(file),
        line(line),
        type(type),
        isLocal(isLocal),
        isDefinition(isDefinition),
        variable(variable),
        declaration(declaration)
  {
  }

  const Metadata *scope;
  const rdcstr *name;
  const rdcstr *linkageName;
  const Metadata *file;
  uint64_t line;
  const Metadata *type;
  bool isLocal;
  bool isDefinition;
  const Metadata *variable;
  const Metadata *declaration;

  virtual rdcstr toString(bool dxcStyleFormatting) const;
};

struct DILocalVariable : public DIBase
{
  static const DIBase::Type DIType = DIBase::LocalVariable;
  DILocalVariable(DW_TAG tag, const Metadata *scope, const rdcstr *name, const Metadata *file,
                  uint64_t line, const Metadata *type, uint64_t arg, DIFlags flags)
      : DIBase(DIType),
        tag(tag),
        scope(scope),
        name(name),
        file(file),
        line(line),
        type(type),
        arg(arg),
        flags(flags)
  {
  }

  DW_TAG tag;
  const Metadata *scope;
  const rdcstr *name;
  const Metadata *file;
  uint64_t line;
  const Metadata *type;
  uint64_t arg;
  DIFlags flags;
  uint64_t alignInBits;

  virtual rdcstr toString(bool dxcStyleFormatting) const;
};

struct DIExpression : public DIBase
{
  static const DIBase::Type DIType = DIBase::Expression;
  DIExpression() : DIBase(DIType) {}
  rdcarray<uint64_t> expr;
  DW_OP op;
  union
  {
    struct
    {
      uint64_t offset;
      uint64_t size;
    } bit_piece;
  } evaluated;

  virtual rdcstr toString(bool dxcStyleFormatting) const;
};

struct DILexicalBlock : public DIBase
{
  static const DIBase::Type DIType = DIBase::LexicalBlock;
  DILexicalBlock(const Metadata *scope, const Metadata *file, uint64_t line, uint64_t column)
      : DIBase(DIType), scope(scope), file(file), line(line), column(column)
  {
  }

  const Metadata *scope;
  const Metadata *file;
  uint64_t line;
  uint64_t column;

  virtual rdcstr toString(bool dxcStyleFormatting) const;
};

struct DISubrange : public DIBase
{
  static const DIBase::Type DIType = DIBase::Subrange;
  DISubrange(int64_t count, int64_t lowerBound)
      : DIBase(DIType), count(count), lowerBound(lowerBound)
  {
  }

  int64_t count;
  int64_t lowerBound;

  virtual rdcstr toString(bool dxcStyleFormatting) const;
};

struct DINamespace : public DIBase
{
  static const DIBase::Type DIType = DIBase::Namespace;
  DINamespace(const Metadata *scope, const Metadata *file, const rdcstr *name, uint64_t line)
      : DIBase(DIType), scope(scope), file(file), name(name), line(line)
  {
  }

  const Metadata *scope;
  const Metadata *file;
  const rdcstr *name;
  uint64_t line;

  virtual rdcstr toString(bool dxcStyleFormatting) const;
};

struct DIImportedEntity : public DIBase
{
  static const DIBase::Type DIType = DIBase::ImportedEntity;
  DIImportedEntity(DW_TAG tag, const Metadata *scope, const Metadata *entity, uint64_t line,
                   const rdcstr *name)
      : DIBase(DIType), tag(tag), scope(scope), entity(entity), line(line), name(name)
  {
  }

  DW_TAG tag;
  const Metadata *scope;
  const Metadata *entity;
  uint64_t line;
  const rdcstr *name;

  virtual rdcstr toString(bool dxcStyleFormatting) const;
};
};    // namespace DXIL

DECLARE_REFLECTION_ENUM(DXIL::DW_LANG);
DECLARE_STRINGISE_TYPE(DXIL::DW_OP);
