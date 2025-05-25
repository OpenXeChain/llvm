//===- PPCCOFFObjectWriter.cpp------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#include "MCTargetDesc/PPCFixupKinds.h"
#include "MCTargetDesc/PPCMCExpr.h"
#include "MCTargetDesc/PPCMCTargetDesc.h"
#include "llvm/BinaryFormat/COFF.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCWinCOFFObjectWriter.h"
#include "llvm/Support/Casting.h"

using namespace llvm;

namespace {

class PPCCOFFObjectWriter : public MCWinCOFFObjectTargetWriter {
public:
  PPCCOFFObjectWriter();

  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsCrossSection,
                        const MCAsmBackend &MAB) const override;
};

} // end anonymous namespace

PPCCOFFObjectWriter::PPCCOFFObjectWriter()
    : MCWinCOFFObjectTargetWriter(COFF::IMAGE_FILE_MACHINE_XBOX360) {}

static MCSymbolRefExpr::VariantKind getAccessVariant(const MCValue &Target,
                                                     const MCFixup &Fixup) {
  const MCExpr *Expr = Fixup.getValue();

  if (Expr->getKind() != MCExpr::Target)
    return Target.getAccessVariant();

  switch (cast<PPCMCExpr>(Expr)->getKind()) {
  case PPCMCExpr::VK_PPC_None:
    return MCSymbolRefExpr::VK_None;
  case PPCMCExpr::VK_PPC_LO:
    return MCSymbolRefExpr::VK_PPC_LO;
  case PPCMCExpr::VK_PPC_HI:
    return MCSymbolRefExpr::VK_PPC_HI;
  case PPCMCExpr::VK_PPC_HA:
    return MCSymbolRefExpr::VK_PPC_HA;
  case PPCMCExpr::VK_PPC_HIGH:
    return MCSymbolRefExpr::VK_PPC_HIGH;
  case PPCMCExpr::VK_PPC_HIGHA:
    return MCSymbolRefExpr::VK_PPC_HIGHA;
  case PPCMCExpr::VK_PPC_HIGHERA:
    return MCSymbolRefExpr::VK_PPC_HIGHERA;
  case PPCMCExpr::VK_PPC_HIGHER:
    return MCSymbolRefExpr::VK_PPC_HIGHER;
  case PPCMCExpr::VK_PPC_HIGHEST:
    return MCSymbolRefExpr::VK_PPC_HIGHEST;
  case PPCMCExpr::VK_PPC_HIGHESTA:
    return MCSymbolRefExpr::VK_PPC_HIGHESTA;
  }
  llvm_unreachable("unknown PPCMCExpr kind");
}

unsigned PPCCOFFObjectWriter::getRelocType(MCContext &Ctx,
                                           const MCValue &Target,
                                           const MCFixup &Fixup,
                                           bool IsCrossSection,
                                           const MCAsmBackend &MAB) const {
  MCFixupKind Kind = Fixup.getKind();
  if (Kind >= FirstLiteralRelocationKind)
    return Kind - FirstLiteralRelocationKind;

  MCSymbolRefExpr::VariantKind Modifier = getAccessVariant(Target, Fixup);

  unsigned Type = 0;
  bool IsPCRel = false;
  if (IsPCRel) {
    switch (Fixup.getTargetKind()) {
    default:
      llvm_unreachable("Unimplemented");
    case PPC::fixup_ppc_br24:
    case PPC::fixup_ppc_br24abs:
    case PPC::fixup_ppc_br24_notoc:
      switch (Modifier) {
      default:
        llvm_unreachable("Unsupported Modifier");
      case MCSymbolRefExpr::VK_None:
        Type = COFF::IMAGE_REL_PPC_REL24; // PC-relative 26-bit branch
        break;
      case MCSymbolRefExpr::VK_PLT:
        Type = COFF::IMAGE_REL_PPC_REL24; // No exact PLT in COFF, fallback
        break;
      case MCSymbolRefExpr::VK_PPC_LOCAL:
        Type = COFF::IMAGE_REL_PPC_REL24; // Closest approximation
        break;
      case MCSymbolRefExpr::VK_PPC_NOTOC:
        Type = COFF::IMAGE_REL_PPC_REL24; // Approximate
        break;
      }
      break;

    case PPC::fixup_ppc_brcond14:
    case PPC::fixup_ppc_brcond14abs:
      Type = COFF::IMAGE_REL_PPC_REL14; // 16-bit PC-relative branch
      break;

    case PPC::fixup_ppc_half16:
      switch (Modifier) {
      default:
        llvm_unreachable("Unsupported Modifier");
      case MCSymbolRefExpr::VK_None:
        Type = COFF::IMAGE_REL_PPC_ADDR16; // 16-bit address
        break;
      case MCSymbolRefExpr::VK_PPC_LO:
        Type = COFF::IMAGE_REL_PPC_REFLO; // Low 16 bits
        break;
      case MCSymbolRefExpr::VK_PPC_HI:
        Type = COFF::IMAGE_REL_PPC_REFHI; // High 16 bits
        break;
      case MCSymbolRefExpr::VK_PPC_HA:
        Type = COFF::IMAGE_REL_PPC_REFHI; // High adjusted (same as HI)
        break;
      }
      break;

    case PPC::fixup_ppc_half16ds:
    case PPC::fixup_ppc_half16dq:
      llvm_unreachable("Invalid PC-relative half16ds relocation");

    case PPC::fixup_ppc_pcrel34:
      switch (Modifier) {
      default:
        llvm_unreachable("Unsupported Modifier for fixup_ppc_pcrel34");
      case MCSymbolRefExpr::VK_PCREL:
        Type = COFF::IMAGE_REL_PPC_REL24; // Approximate as REL24
        break;
      case MCSymbolRefExpr::VK_PPC_GOT_PCREL:
        Type = COFF::IMAGE_REL_PPC_TOCREL16; // Use TOCREL16 as GOT relative
                                             // approx.
        break;
      case MCSymbolRefExpr::VK_PPC_GOT_TLSGD_PCREL:
      case MCSymbolRefExpr::VK_PPC_GOT_TLSLD_PCREL:
      case MCSymbolRefExpr::VK_PPC_GOT_TPREL_PCREL:
        Type = COFF::IMAGE_REL_PPC_TOCREL16; // Approximate all as TOCREL16
        break;
      }
      break;

    case FK_Data_4:
    case FK_PCRel_4:
      Type = COFF::IMAGE_REL_PPC_ADDR32; // 32-bit address
      break;

    case FK_Data_8:
    case FK_PCRel_8:
      Type = COFF::IMAGE_REL_PPC_ADDR64; // 64-bit address
      break;
    }
  } else {
    switch (Fixup.getTargetKind()) {
    default:
      llvm_unreachable("invalid fixup kind!");
    case PPC::fixup_ppc_br24abs:
      Type = COFF::IMAGE_REL_PPC_ADDR24; // 26-bit absolute address
      break;
    case PPC::fixup_ppc_brcond14abs:
      Type = COFF::IMAGE_REL_PPC_ADDR14; // 16-bit shifted left 2
      break;
    case PPC::fixup_ppc_half16:
      switch (Modifier) {
      default:
        llvm_unreachable("Unsupported Modifier");
      case MCSymbolRefExpr::VK_None:
        Type = COFF::IMAGE_REL_PPC_ADDR16;
        break;
      case MCSymbolRefExpr::VK_PPC_LO:
        Type = COFF::IMAGE_REL_PPC_REFLO;
        break;
      case MCSymbolRefExpr::VK_PPC_HI:
        Type = COFF::IMAGE_REL_PPC_REFHI;
        break;
      case MCSymbolRefExpr::VK_PPC_HA:
        Type = COFF::IMAGE_REL_PPC_REFHI; // no separate HA in COFF, use HI
        break;
        // Other high modifiers not available in COFF
      }
      break;

    case PPC::fixup_ppc_half16ds:
    case PPC::fixup_ppc_half16dq:
      switch (Modifier) {
      default:
        llvm_unreachable("Unsupported Modifier");
      case MCSymbolRefExpr::VK_None:
        Type = COFF::IMAGE_REL_PPC_ADDR16; // No DS in COFF, fallback
        break;
      case MCSymbolRefExpr::VK_PPC_LO:
        Type = COFF::IMAGE_REL_PPC_REFLO;
        break;
      case MCSymbolRefExpr::VK_GOT:
        Type = COFF::IMAGE_REL_PPC_TOCREL16; // GOT approx TOC relative
        break;
      case MCSymbolRefExpr::VK_PPC_GOT_LO:
        Type = COFF::IMAGE_REL_PPC_TOCREL16; // Approximate
        break;
      case MCSymbolRefExpr::VK_PPC_TOC:
        Type = COFF::IMAGE_REL_PPC_TOCREL16;
        break;
      case MCSymbolRefExpr::VK_PPC_TOC_LO:
        Type = COFF::IMAGE_REL_PPC_TOCREL16;
        break;
      case MCSymbolRefExpr::VK_TPREL:
        Type = COFF::IMAGE_REL_PPC_SECREL16; // Section relative 16-bit
        break;
      case MCSymbolRefExpr::VK_PPC_TPREL_LO:
        Type = COFF::IMAGE_REL_PPC_SECRELLO; // Low 16 bit section relative
        break;
      case MCSymbolRefExpr::VK_DTPREL:
        Type = COFF::IMAGE_REL_PPC_SECREL16; // Approximate TLS section relative
        break;
      case MCSymbolRefExpr::VK_PPC_DTPREL_LO:
        Type = COFF::IMAGE_REL_PPC_SECRELLO;
        break;
        // Other modifiers can be approximated similarly
      }
      break;

    case PPC::fixup_ppc_nofixup:
      switch (Modifier) {
      default:
        llvm_unreachable("Unsupported Modifier");
      case MCSymbolRefExpr::VK_PPC_TLSGD:
      case MCSymbolRefExpr::VK_PPC_TLSLD:
      case MCSymbolRefExpr::VK_PPC_TLS:
      case MCSymbolRefExpr::VK_PPC_TLS_PCREL:
        Type =
            COFF::IMAGE_REL_PPC_SECREL; // Section relative as TLS approximation
        break;
      }
      break;

    case PPC::fixup_ppc_imm34:
      switch (Modifier) {
      default:
        report_fatal_error("Unsupported Modifier for fixup_ppc_imm34.");
      case MCSymbolRefExpr::VK_DTPREL:
      case MCSymbolRefExpr::VK_TPREL:
        Type = COFF::IMAGE_REL_PPC_ADDR64; // Use 64-bit address for 34-bit
                                           // immediate
        break;
      }
      break;

    case FK_Data_8:
      switch (Modifier) {
      default:
        Type = COFF::IMAGE_REL_PPC_ADDR64;
        break;
      case MCSymbolRefExpr::VK_PPC_TOCBASE:
        Type =
            COFF::IMAGE_REL_PPC_TOCREL16; // TOC base approximated as TOCREL16
        break;
      }
      break;

    case FK_Data_4:
      switch (Modifier) {
      case MCSymbolRefExpr::VK_DTPREL:
        Type = COFF::IMAGE_REL_PPC_SECREL; // Section relative
        break;
      default:
        Type = COFF::IMAGE_REL_PPC_ADDR32;
      }
      break;

    case FK_Data_2:
      Type = COFF::IMAGE_REL_PPC_ADDR16;
      break;
    }
  }

  return Type;
}

std::unique_ptr<MCObjectTargetWriter> llvm::createPPCCOFFObjectWriter() {
  return std::make_unique<PPCCOFFObjectWriter>();
}
