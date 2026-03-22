# Node Wizard — TODO

---

### 1. Update firmware_write Prototype

**Issue:** The `firmware_write` callback signature changed to include a `write_result_t` completion callback parameter. The Node Wizard code generator still emits the old two-parameter signature.

**Scope:** Update the Node Wizard template for `firmware_write` to generate:
```c
void CallbacksConfigMem_firmware_write(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info, write_result_t write_result)
```
and call `write_result(statemachine_info, config_mem_write_request_info, true/false)` in the generated stub body.

---
