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

    switch (Fixup.getTargetKind()) {
    default:
      llvm_unreachable(std::string("Unimplemented PPC32 COFF fixup kind " + std::to_string(Fixup.getTargetKind())).c_str());
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


  return Type;
}

std::unique_ptr<MCObjectTargetWriter> llvm::createPPCCOFFObjectWriter() {
  return std::make_unique<PPCCOFFObjectWriter>();
}
