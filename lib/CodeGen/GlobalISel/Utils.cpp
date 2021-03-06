//===- llvm/CodeGen/GlobalISel/Utils.cpp -------------------------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file This file implements the utility functions used by the GlobalISel
/// pipeline.
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/GlobalISel/Utils.h"
#include "llvm/ADT/Twine.h"
#include "llvm/CodeGen/GlobalISel/RegisterBankInfo.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineOptimizationRemarkEmitter.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/IR/Constants.h"

#define DEBUG_TYPE "globalisel-utils"

using namespace llvm;

unsigned llvm::constrainRegToClass(MachineRegisterInfo &MRI,
                                   const TargetInstrInfo &TII,
                                   const RegisterBankInfo &RBI,
                                   MachineInstr &InsertPt, unsigned Reg,
                                   const TargetRegisterClass &RegClass) {
  if (!RBI.constrainGenericRegister(Reg, RegClass, MRI)) {
    unsigned NewReg = MRI.createVirtualRegister(&RegClass);
    BuildMI(*InsertPt.getParent(), InsertPt, InsertPt.getDebugLoc(),
            TII.get(TargetOpcode::COPY), NewReg)
        .addReg(Reg);
    return NewReg;
  }

  return Reg;
}


unsigned llvm::constrainOperandRegClass(
    const MachineFunction &MF, const TargetRegisterInfo &TRI,
    MachineRegisterInfo &MRI, const TargetInstrInfo &TII,
    const RegisterBankInfo &RBI, MachineInstr &InsertPt, const MCInstrDesc &II,
    unsigned Reg, unsigned OpIdx) {
  // Assume physical registers are properly constrained.
  assert(TargetRegisterInfo::isVirtualRegister(Reg) &&
         "PhysReg not implemented");

  const TargetRegisterClass *RegClass = TII.getRegClass(II, OpIdx, &TRI, MF);
  // Some of the target independent instructions, like COPY, may not impose any
  // register class constraints on some of their operands:
  if (!RegClass) {
    assert(!isTargetSpecificOpcode(II.getOpcode()) &&
           "Only target independent instructions are allowed to have operands "
           "with no register class constraints");
    // FIXME: Just bailing out like this here could be not enough, unless we
    // expect the users of this function to do the right thing for PHIs and
    // COPY:
    //   v1 = COPY v0
    //   v2 = COPY v1
    // v1 here may end up not being constrained at all. Please notice that to
    // reproduce the issue we likely need a destination pattern of a selection
    // rule producing such extra copies, not just an input GMIR with them as
    // every existing target using selectImpl handles copies before calling it
    // and they never reach this function.
    return Reg;
  }
  return constrainRegToClass(MRI, TII, RBI, InsertPt, Reg, *RegClass);
}

bool llvm::constrainSelectedInstRegOperands(MachineInstr &I,
                                            const TargetInstrInfo &TII,
                                            const TargetRegisterInfo &TRI,
                                            const RegisterBankInfo &RBI) {
  assert(!isPreISelGenericOpcode(I.getOpcode()) &&
         "A selected instruction is expected");
  MachineBasicBlock &MBB = *I.getParent();
  MachineFunction &MF = *MBB.getParent();
  MachineRegisterInfo &MRI = MF.getRegInfo();

  for (unsigned OpI = 0, OpE = I.getNumExplicitOperands(); OpI != OpE; ++OpI) {
    MachineOperand &MO = I.getOperand(OpI);

    // There's nothing to be done on non-register operands.
    if (!MO.isReg())
      continue;

    DEBUG(dbgs() << "Converting operand: " << MO << '\n');
    assert(MO.isReg() && "Unsupported non-reg operand");

    unsigned Reg = MO.getReg();
    // Physical registers don't need to be constrained.
    if (TRI.isPhysicalRegister(Reg))
      continue;

    // Register operands with a value of 0 (e.g. predicate operands) don't need
    // to be constrained.
    if (Reg == 0)
      continue;

    // If the operand is a vreg, we should constrain its regclass, and only
    // insert COPYs if that's impossible.
    // constrainOperandRegClass does that for us.
    MO.setReg(constrainOperandRegClass(MF, TRI, MRI, TII, RBI, I, I.getDesc(),
                                       Reg, OpI));

    // Tie uses to defs as indicated in MCInstrDesc if this hasn't already been
    // done.
    if (MO.isUse()) {
      int DefIdx = I.getDesc().getOperandConstraint(OpI, MCOI::TIED_TO);
      if (DefIdx != -1 && !I.isRegTiedToUseOperand(DefIdx))
        I.tieOperands(DefIdx, OpI);
    }
  }
  return true;
}

bool llvm::isTriviallyDead(const MachineInstr &MI,
                           const MachineRegisterInfo &MRI) {
  // If we can move an instruction, we can remove it.  Otherwise, it has
  // a side-effect of some sort.
  bool SawStore = false;
  if (!MI.isSafeToMove(/*AA=*/nullptr, SawStore))
    return false;

  // Instructions without side-effects are dead iff they only define dead vregs.
  for (auto &MO : MI.operands()) {
    if (!MO.isReg() || !MO.isDef())
      continue;

    unsigned Reg = MO.getReg();
    if (TargetRegisterInfo::isPhysicalRegister(Reg) ||
        !MRI.use_nodbg_empty(Reg))
      return false;
  }
  return true;
}

void llvm::reportGISelFailure(MachineFunction &MF, const TargetPassConfig &TPC,
                              MachineOptimizationRemarkEmitter &MORE,
                              MachineOptimizationRemarkMissed &R) {
  MF.getProperties().set(MachineFunctionProperties::Property::FailedISel);

  // Print the function name explicitly if we don't have a debug location (which
  // makes the diagnostic less useful) or if we're going to emit a raw error.
  if (!R.getLocation().isValid() || TPC.isGlobalISelAbortEnabled())
    R << (" (in function: " + MF.getName() + ")").str();

  if (TPC.isGlobalISelAbortEnabled())
    report_fatal_error(R.getMsg());
  else
    MORE.emit(R);
}

void llvm::reportGISelFailure(MachineFunction &MF, const TargetPassConfig &TPC,
                              MachineOptimizationRemarkEmitter &MORE,
                              const char *PassName, StringRef Msg,
                              const MachineInstr &MI) {
  MachineOptimizationRemarkMissed R(PassName, "GISelFailure: ",
                                    MI.getDebugLoc(), MI.getParent());
  R << Msg;
  // Printing MI is expensive;  only do it if expensive remarks are enabled.
  if (MORE.allowExtraAnalysis(PassName))
    R << ": " << ore::MNV("Inst", MI);
  reportGISelFailure(MF, TPC, MORE, R);
}

Optional<int64_t> llvm::getConstantVRegVal(unsigned VReg,
                                           const MachineRegisterInfo &MRI) {
  MachineInstr *MI = MRI.getVRegDef(VReg);
  if (MI->getOpcode() != TargetOpcode::G_CONSTANT)
    return None;

  if (MI->getOperand(1).isImm())
    return MI->getOperand(1).getImm();

  if (MI->getOperand(1).isCImm() &&
      MI->getOperand(1).getCImm()->getBitWidth() <= 64)
    return MI->getOperand(1).getCImm()->getSExtValue();

  return None;
}

const llvm::ConstantFP* llvm::getConstantFPVRegVal(unsigned VReg,
                                       const MachineRegisterInfo &MRI) {
  MachineInstr *MI = MRI.getVRegDef(VReg);
  if (TargetOpcode::G_FCONSTANT != MI->getOpcode())
    return nullptr;
  return MI->getOperand(1).getFPImm();
}

llvm::MachineInstr *llvm::getOpcodeDef(unsigned Opcode, unsigned Reg,
                                       const MachineRegisterInfo &MRI) {
  auto *DefMI = MRI.getVRegDef(Reg);
  auto DstTy = MRI.getType(DefMI->getOperand(0).getReg());
  if (!DstTy.isValid())
    return nullptr;
  while (DefMI->getOpcode() == TargetOpcode::COPY) {
    unsigned SrcReg = DefMI->getOperand(1).getReg();
    auto SrcTy = MRI.getType(SrcReg);
    if (!SrcTy.isValid() || SrcTy != DstTy)
      break;
    DefMI = MRI.getVRegDef(SrcReg);
  }
  return DefMI->getOpcode() == Opcode ? DefMI : nullptr;
}
