/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/slab.h>
#include <linux/diagchar.h>
#include <linux/platform_device.h>
#include <linux/kmemleak.h>
#include "diagchar.h"
#include "diagfwd.h"
#include "diagfwd_cntl.h"
static uint16_t reg_dirty;
#define HDR_SIZ 8

void diag_clean_modem_reg_fn(struct work_struct *work)
{
	pr_debug("diag: clean modem registration\n");
	reg_dirty |= DIAG_CON_MPSS;
	diag_clear_reg(MODEM_PROC);
	reg_dirty ^= DIAG_CON_MPSS;
}

void diag_clean_lpass_reg_fn(struct work_struct *work)
{
	pr_debug("diag: clean lpass registration\n");
	reg_dirty |= DIAG_CON_LPASS;
	diag_clear_reg(LPASS_PROC);
	reg_dirty ^= DIAG_CON_LPASS;
}

void diag_clean_wcnss_reg_fn(struct work_struct *work)
{
	pr_debug("diag: clean wcnss registration\n");
	reg_dirty |= DIAG_CON_WCNSS;
	diag_clear_reg(WCNSS_PROC);
	reg_dirty ^= DIAG_CON_WCNSS;
}

void diag_smd_cntl_notify(void *ctxt, unsigned event)
{
	int r1, r2;

	if (!(driver->ch_cntl))
		return;

	switch (event) {
	case SMD_EVENT_DATA:
		r1 = smd_read_avail(driver->ch_cntl);
		r2 = smd_cur_packet_size(driver->ch_cntl);
		if (r1 > 0 && r1 == r2)
			queue_work(driver->diag_wq,
				 &(driver->diag_read_smd_cntl_work));
		else
			pr_debug("diag: incomplete pkt on Modem CNTL ch\n");
		break;
	case SMD_EVENT_OPEN:
		queue_work(driver->diag_cntl_wq,
			 &(driver->diag_modem_mask_update_work));
		break;
	}
}

void diag_smd_lpass_cntl_notify(void *ctxt, unsigned event)
{
	int r1, r2;

	if (!(driver->chlpass_cntl))
		return;

	switch (event) {
	case SMD_EVENT_DATA:
		r1 = smd_read_avail(driver->chlpass_cntl);
		r2 = smd_cur_packet_size(driver->chlpass_cntl);
		if (r1 > 0 && r1 == r2)
			queue_work(driver->diag_wq,
				 &(driver->diag_read_smd_lpass_cntl_work));
		else
			pr_debug("diag: incomplete pkt on LPASS CNTL ch\n");
		break;
	case SMD_EVENT_OPEN:
		queue_work(driver->diag_cntl_wq,
			 &(driver->diag_lpass_mask_update_work));
		break;
	}
}

void diag_smd_wcnss_cntl_notify(void *ctxt, unsigned event)
{
	int r1, r2;

	if (!(driver->ch_wcnss_cntl))
		return;

	switch (event) {
	case SMD_EVENT_DATA:
		r1 = smd_read_avail(driver->ch_wcnss_cntl);
		r2 = smd_cur_packet_size(driver->ch_wcnss_cntl);
		if (r1 > 0 && r1 == r2)
			queue_work(driver->diag_wq,
				 &(driver->diag_read_smd_wcnss_cntl_work));
		else
			pr_debug("diag: incomplete pkt on WCNSS CNTL ch\n");
		break;
	case SMD_EVENT_OPEN:
		queue_work(driver->diag_cntl_wq,
			 &(driver->diag_wcnss_mask_update_work));
		break;
	}
}

static void diag_smd_cntl_send_req(int proc_num)
{
	int data_len = 0, type = -1, count_bytes = 0, j, r, flag = 0;
	struct bindpkt_params_per_process *pkt_params =
		 kzalloc(sizeof(struct bindpkt_params_per_process), GFP_KERNEL);
	struct diag_ctrl_msg *msg;
	struct cmd_code_range *range;
	struct bindpkt_params *temp;
	void *buf = NULL;
	smd_channel_t *smd_ch = NULL;
	
	uint16_t reg_mask = 0;

	if (pkt_params == NULL) {
		pr_alert("diag: Memory allocation failure\n");
		return;
	}

	if (proc_num == MODEM_PROC) {
		buf = driver->buf_in_cntl;
		smd_ch = driver->ch_cntl;
		reg_mask = DIAG_CON_MPSS;
	} else if (proc_num == LPASS_PROC) {
		buf = driver->buf_in_lpass_cntl;
		smd_ch = driver->chlpass_cntl;
		reg_mask = DIAG_CON_LPASS;
	} else if (proc_num == WCNSS_PROC) {
		buf = driver->buf_in_wcnss_cntl;
		smd_ch = driver->ch_wcnss_cntl;
		reg_mask = DIAG_CON_WCNSS;
	}

	if (!smd_ch || !buf) {
		kfree(pkt_params);
		return;
	}

	while (count_bytes + HDR_SIZ <= total_recd) {
		type = *(uint32_t *)(buf);
		data_len = *(uint32_t *)(buf + 4);
		if (type < DIAG_CTRL_MSG_REG ||
				 type > DIAG_CTRL_MSG_LAST) {
			pr_alert("diag: In %s, Invalid Msg type %d proc %d",
				 __func__, type, smd_info->peripheral);
			break;
		}
		if (data_len < 0 || data_len > total_recd) {
			pr_alert("diag: In %s, Invalid data len %d, total_recd: %d, proc %d",
				 __func__, data_len, total_recd,
				 smd_info->peripheral);
			break;
		}
		count_bytes = count_bytes+HDR_SIZ+data_len;
		if (type == DIAG_CTRL_MSG_REG && total_recd >= count_bytes) {
			msg = buf+HDR_SIZ;
			range = buf+HDR_SIZ+
					sizeof(struct diag_ctrl_msg);
			pkt_params->count = msg->count_entries;
			pkt_params->params = kzalloc(pkt_params->count *
				sizeof(struct bindpkt_params), GFP_KERNEL);
			if (ZERO_OR_NULL_PTR(pkt_params->params)) {
				pr_alert("diag: In %s, Memory alloc fail\n",
					__func__);
				kfree(pkt_params);
				return flag;
			}
			count_bytes = count_bytes+HDR_SIZ+data_len;
			if (type == DIAG_CTRL_MSG_REG && r >= count_bytes) {
				msg = buf+HDR_SIZ;
				range = buf+HDR_SIZ+
						sizeof(struct diag_ctrl_msg);
				pkt_params->count = msg->count_entries;
				temp = kzalloc(pkt_params->count * sizeof(struct
						 bindpkt_params), GFP_KERNEL);
				if (temp == NULL) {
					pr_alert("diag: Memory alloc fail\n");
					kfree(pkt_params);
					return;
				}
				for (j = 0; j < pkt_params->count; j++) {
					temp->cmd_code = msg->cmd_code;
					temp->subsys_id = msg->subsysid;
					temp->client_id = proc_num;
					temp->proc_id = proc_num;
					temp->cmd_code_lo = range->cmd_code_lo;
					temp->cmd_code_hi = range->cmd_code_hi;
					range++;
					temp++;
				}
				temp -= pkt_params->count;
				pkt_params->params = temp;
				flag = 1;
				if (!(reg_dirty & reg_mask))
					diagchar_ioctl(NULL,
					 DIAG_IOCTL_COMMAND_REG, (unsigned long)
								pkt_params);
				else
					pr_err("diag: drop reg proc %d\n",
								 proc_num);
				kfree(temp);
			} else if (type != DIAG_CTRL_MSG_REG) {
				flag = 1;
			}
			buf = buf + HDR_SIZ + data_len;
		}
	}
	kfree(pkt_params);
	if (flag) {
		
		if (proc_num == MODEM_PROC)
			diag_smd_cntl_notify(NULL, SMD_EVENT_DATA);
		else if (proc_num == LPASS_PROC)
			diag_smd_lpass_cntl_notify(NULL, SMD_EVENT_DATA);
		else if (proc_num == WCNSS_PROC)
			diag_smd_wcnss_cntl_notify(NULL, SMD_EVENT_DATA);
	}
}

void diag_read_smd_cntl_work_fn(struct work_struct *work)
{
	diag_smd_cntl_send_req(MODEM_PROC);
}

void diag_read_smd_lpass_cntl_work_fn(struct work_struct *work)
{
	struct diag_ctrl_msg_diagmode diagmode;
	char buf[sizeof(struct diag_ctrl_msg_diagmode)];
	int msg_size = sizeof(struct diag_ctrl_msg_diagmode);
	int wr_size = -ENOMEM, retry_count = 0, timer;
	struct diag_smd_info *data = NULL;

	/* For now only allow the modem to receive the message */
	if (!smd_info || smd_info->type != SMD_CNTL_TYPE ||
		(smd_info->peripheral != MODEM_DATA))
		return;

	data = &driver->smd_data[smd_info->peripheral];
	if (!data)
		return;

	mutex_lock(&driver->diag_cntl_mutex);
	diagmode.ctrl_pkt_id = DIAG_CTRL_MSG_DIAGMODE;
	diagmode.ctrl_pkt_data_len = 36;
	diagmode.version = 1;
	diagmode.sleep_vote = real_time ? 1 : 0;
	/*
	 * 0 - Disables real-time logging (to prevent
	 *     frequent APPS wake-ups, etc.).
	 * 1 - Enable real-time logging
	 */
	diagmode.real_time = real_time;
	diagmode.use_nrt_values = 0;
	diagmode.commit_threshold = 0;
	diagmode.sleep_threshold = 0;
	diagmode.sleep_time = 0;
	diagmode.drain_timer_val = 0;
	diagmode.event_stale_timer_val = 0;

	memcpy(buf, &diagmode, msg_size);

	if (smd_info->ch) {
		while (retry_count < 3) {
			wr_size = smd_write(smd_info->ch, buf, msg_size);
			if (wr_size == -ENOMEM) {
				/*
				 * The smd channel is full. Delay while
				 * smd processes existing data and smd
				 * has memory become available. The delay
				 * of 2000 was determined empirically as
				 * best value to use.
				 */
				retry_count++;
				for (timer = 0; timer < 5; timer++)
					udelay(2000);
			} else {
				data =
				&driver->smd_data[smd_info->peripheral];
				driver->real_time_mode = real_time;
				break;
			}
		}
		if (wr_size != msg_size)
			pr_err("diag: proc %d fail feature update %d, tried %d",
				smd_info->peripheral,
				wr_size, msg_size);
	} else {
		pr_err("diag: ch invalid, feature update on proc %d\n",
				smd_info->peripheral);
	}
	process_lock_enabling(&data->nrt_lock, real_time);

void diag_read_smd_wcnss_cntl_work_fn(struct work_struct *work)
{
	diag_smd_cntl_send_req(WCNSS_PROC);
}

static int diag_smd_cntl_probe(struct platform_device *pdev)
{
	int r = 0;

	
	if (chk_apps_only()) {
		if (pdev->id == SMD_APPS_MODEM)
			r = smd_open("DIAG_CNTL", &driver->ch_cntl, driver,
							diag_smd_cntl_notify);
		if (pdev->id == SMD_APPS_QDSP)
			r = smd_named_open_on_edge("DIAG_CNTL", SMD_APPS_QDSP
					, &driver->chlpass_cntl, driver,
					diag_smd_lpass_cntl_notify);
		if (pdev->id == SMD_APPS_WCNSS)
			r = smd_named_open_on_edge("APPS_RIVA_CTRL",
				SMD_APPS_WCNSS, &driver->ch_wcnss_cntl,
					driver, diag_smd_wcnss_cntl_notify);
		pr_debug("diag: open CNTL port, ID = %d,r = %d\n", pdev->id, r);
	}
	return 0;
}

static int diagfwd_cntl_runtime_suspend(struct device *dev)
{
	dev_dbg(dev, "pm_runtime: suspending...\n");
	return 0;
}

static int diagfwd_cntl_runtime_resume(struct device *dev)
{
	dev_dbg(dev, "pm_runtime: resuming...\n");
	return 0;
}

static const struct dev_pm_ops diagfwd_cntl_dev_pm_ops = {
	.runtime_suspend = diagfwd_cntl_runtime_suspend,
	.runtime_resume = diagfwd_cntl_runtime_resume,
};

static struct platform_driver msm_smd_ch1_cntl_driver = {

	.probe = diag_smd_cntl_probe,
	.driver = {
			.name = "DIAG_CNTL",
			.owner = THIS_MODULE,
			.pm   = &diagfwd_cntl_dev_pm_ops,
		   },
};

static struct platform_driver diag_smd_lite_cntl_driver = {

	.probe = diag_smd_cntl_probe,
	.driver = {
			.name = "APPS_RIVA_CTRL",
			.owner = THIS_MODULE,
			.pm   = &diagfwd_cntl_dev_pm_ops,
		   },
};

void diagfwd_cntl_init(void)
{
	reg_dirty = 0;
	driver->polling_reg_flag = 0;
	driver->diag_cntl_wq = create_singlethread_workqueue("diag_cntl_wq");
	if (driver->buf_in_cntl == NULL) {
		driver->buf_in_cntl = kzalloc(IN_BUF_SIZE, GFP_KERNEL);
		if (driver->buf_in_cntl == NULL)
			goto err;
		kmemleak_not_leak(driver->buf_in_cntl);
	}
	if (driver->buf_in_lpass_cntl == NULL) {
		driver->buf_in_lpass_cntl = kzalloc(IN_BUF_SIZE, GFP_KERNEL);
		if (driver->buf_in_lpass_cntl == NULL)
			goto err;
		kmemleak_not_leak(driver->buf_in_lpass_cntl);
	}
	if (driver->buf_in_wcnss_cntl == NULL) {
		driver->buf_in_wcnss_cntl = kzalloc(IN_BUF_SIZE, GFP_KERNEL);
		if (driver->buf_in_wcnss_cntl == NULL)
			goto err;
		kmemleak_not_leak(driver->buf_in_wcnss_cntl);
	}
	platform_driver_register(&msm_smd_ch1_cntl_driver);
	platform_driver_register(&diag_smd_lite_cntl_driver);

	return;
err:
		pr_err("diag: Could not initialize diag buffers");
		kfree(driver->buf_in_cntl);
		kfree(driver->buf_in_lpass_cntl);
		kfree(driver->buf_in_wcnss_cntl);
		if (driver->diag_cntl_wq)
			destroy_workqueue(driver->diag_cntl_wq);
}

void diagfwd_cntl_exit(void)
{
	smd_close(driver->ch_cntl);
	smd_close(driver->chlpass_cntl);
	smd_close(driver->ch_wcnss_cntl);
	driver->ch_cntl = 0;
	driver->chlpass_cntl = 0;
	driver->ch_wcnss_cntl = 0;
	destroy_workqueue(driver->diag_cntl_wq);
	platform_driver_unregister(&msm_smd_ch1_cntl_driver);
	platform_driver_unregister(&diag_smd_lite_cntl_driver);

	kfree(driver->buf_in_cntl);
	kfree(driver->buf_in_lpass_cntl);
	kfree(driver->buf_in_wcnss_cntl);
}
