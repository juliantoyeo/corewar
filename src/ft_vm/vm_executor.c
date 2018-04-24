/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   vm_executor.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rnugroho <rnugroho@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/04/23 11:23:54 by rnugroho          #+#    #+#             */
/*   Updated: 2018/04/24 01:05:49 by rnugroho         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_vm.h"

int
	vm_checker_oc(t_op *op)
{
	int param[3];
	int i;

	if (!g_op_dict[op->opcode].is_oc)
		return (0);
	param[0] = (op->oc & 192) >> 6;
	param[1] = (op->oc & 48) >> 4;
	param[2] = (op->oc & 12) >> 2;
	i = 0;
	while (i != 3)
	{
		if (param[i] == IND_CODE)
			if (!(g_op_dict[op->opcode].p_type[i] & T_IND))
				return (-1);
		if (param[i] == DIR_CODE)
			if (!(g_op_dict[op->opcode].p_type[i] & T_DIR))
				return (-1);
		if (param[i] == REG_CODE)
			if (!(g_op_dict[op->opcode].p_type[i] & T_REG))
				return (-1);
		if (param[i] == 0)
			if (g_op_dict[op->opcode].p_type[i])
				return (-1);
		i++;
	}
	return (0);
}

int
	vm_decompiler_param(t_process *p, t_op *op)
{
	int		i;

	i = 0;
	op->param_c = g_op_dict[op->opcode].param_c;
	while (i < op->param_c)
	{
		if (vm_checker_oc(op) == -1)
			return (-1);
		op->params[i].type = (g_op_dict[op->opcode].is_oc) ?
		(op->oc & (0xC0 >> (i * 2))) >> ((3 - i) * 2) : op->oc;
		(op->params[i].type == REG_CODE) ? op->params[i].size = 1 : 0;
		(op->params[i].type == IND_CODE) ? op->params[i].size = 2 : 0;
		(op->params[i].type == DIR_CODE) ? op->params[i].size =
			g_op_dict[op->opcode].d_size : 0;
		op->params[i].value =
		vm_binary_toint(&g_memory[p->offset + p->pc + op->size],
			op->params[i].size);
		op->size += op->params[i].size;
		i++;
	}
	return (0);
}

int
	vm_decompiler_op(t_vm *vm, t_process *p, t_op *op)
{
	(void)vm;
	op->opcode = g_memory[p->offset + p->pc];
	op->size += g_op_dict[op->opcode].is_oc ? 2 : 1;
	if (op->opcode < 0x01 || op->opcode > 0x10)
		return (-1);
	if (g_op_dict[op->opcode].is_oc)
		op->oc = g_memory[p->offset + p->pc + 1];
	else
		op->oc = g_op_dict[op->opcode].p_type[0];
	if (vm_decompiler_param(p, op) == -1)
		return (-1);
	p->pc += op->size;
	return (0);
}

void
	vm_executor_process(t_vm *vm, t_process *p)
{
	t_op op;

	ft_bzero(&op, sizeof(t_op));
	if (vm_decompiler_op(vm, p, &op) == -1)
	{
		ft_bzero(&p->op, sizeof(t_op));
		vm_op_inc(vm, p, NULL);
	}
	p->op = op;
}

void
	vm_executor(t_vm *vm)
{
	int		i;
	int		j;

	i = vm->champ_size - 1;
	while (i >= 0)
	{
		j = -1;
		while (++j < (int)(vm->champ[i].processes->size))
			vm_executor_process(vm,
				&(((t_process*)vm->champ[i].processes->data)[j]));
		i--;
	}
}
