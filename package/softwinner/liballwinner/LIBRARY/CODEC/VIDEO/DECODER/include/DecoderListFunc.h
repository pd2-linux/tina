/*
* Cedarx framework.
* Copyright (c) 2008-2015 Allwinner Technology Co. Ltd.
* Copyright (c) 2014 BZ Chen <bzchen@allwinnertech.com>
*
* This file is part of Cedarx.
*
* Cedarx is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This program is distributed "as is" WITHOUT ANY WARRANTY of any
* kind, whether express or implied; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*/

CDX_INTERFACE void __DecoderListAdd(struct DecoderListNodeS *new,
                        struct DecoderListNodeS *prev, struct DecoderListNodeS *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

CDX_INTERFACE void DecoderListAdd(struct DecoderListNodeS *new, struct DecoderListS *list)
{
	__DecoderListAdd(new, (struct DecoderListNodeS *)list, list->head);
}

CDX_INTERFACE void DecoderListAddBefore(struct DecoderListNodeS *new, 
                                    struct DecoderListNodeS *pos)
{
	__DecoderListAdd(new, pos->prev, pos);
}

CDX_INTERFACE void DecoderListAddAfter(struct DecoderListNodeS *new, 
                                    struct DecoderListNodeS *pos)
{
	__DecoderListAdd(new, pos, pos->next);
}

CDX_INTERFACE void DecoderListAddTail(struct DecoderListNodeS *new, struct DecoderListS *list)
{
	__DecoderListAdd(new, list->tail, (struct DecoderListNodeS *)list);
}

CDX_INTERFACE void __DecoderListDel(struct DecoderListNodeS *prev, struct DecoderListNodeS *next)
{
	next->prev = prev;
	prev->next = next;
}

CDX_INTERFACE void DecoderListDel(struct DecoderListNodeS *node)
{
	__DecoderListDel(node->prev, node->next);
	node->next = CDX_LIST_POISON1;
	node->prev = CDX_LIST_POISON2;
}

CDX_INTERFACE int DecoderListEmpty(const struct DecoderListS *list)
{
	return (list->head == (struct DecoderListNodeS *)list) 
	       && (list->tail == (struct DecoderListNodeS *)list);
}

