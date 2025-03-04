var common_stmts = require("common_statements");

if (mysqld.global.transaction_count === undefined) {
  mysqld.global.transaction_count = 0;
}

// at start, .connects is undefined
// at first connect, set it to 0
// at each following connect, increment it.
//
// .globals is shared between mock-server threads
if (mysqld.global.connects === undefined) {
  mysqld.global.connects = 0;
} else {
  mysqld.global.connects = mysqld.global.connects + 1;
}

if (mysqld.global.update_attributes_count === undefined) {
  mysqld.global.update_attributes_count = 0;
}

if (mysqld.global.update_last_check_in_count === undefined) {
  mysqld.global.update_last_check_in_count = 0;
}

if (mysqld.global.metadata_schema_version === undefined) {
  mysqld.global.metadata_schema_version = [2, 2, 0];
}

if (mysqld.global.routing_guidelines === undefined) {
  mysqld.global.routing_guidelines = "";
}

({
  handshake: {
    greeting: {
      server_version: mysqld.global.server_version,
    },
  },
  stmts: function(stmt) {
    if (mysqld.global.server_version === undefined) {
      // Let's keep the default server version as some known compatible version.
      // If there is a need to some specific compatibility checks, this should
      // be overwritten from the test.
      mysqld.global.server_version = "8.3.0";
    }

    var options = {
      cluster_type: "gr",
      metadata_schema_version: mysqld.global.metadata_schema_version,
      clusterset_present: 1,
      bootstrap_target_type: "clusterset",
      clusterset_target_cluster_id: mysqld.global.target_cluster_id,
      view_id: mysqld.global.view_id,
      clusterset_data: mysqld.global.clusterset_data,
      router_options: mysqld.global.router_options,
      clusterset_simulate_cluster_not_found:
          mysqld.global.simulate_cluster_not_found,
      routing_guidelines: mysqld.global.routing_guidelines,
    };

    // TODO: clean those not needed here
    var common_responses = common_stmts.prepare_statement_responses(
        [
          "router_set_session_options",
          "router_set_gr_consistency_level",
          "router_select_schema_version",
          "router_select_cluster_type_v2",
          "router_count_clusters_v2",
          "router_check_member_state",
          "router_select_members_count",
          "router_select_replication_group_name",
          "router_show_cipher_status",
          "router_select_cluster_instances_v2_gr",
          "router_select_router_options_view",
          "router_commit",
          "router_rollback",
          "get_guidelines_router_info",
          "get_routing_guidelines",
          "get_routing_guidelines_version",

          "select_port",

          // clusterset specific
          "router_clusterset_view_id",
          "router_clusterset_all_nodes_by_clusterset_id",
          "router_clusterset_present",
          "router_bootstrap_target_type",
          "router_clusterset_select_cluster_info_by_primary_role",
          "router_clusterset_select_cluster_info_by_gr_uuid",
          "router_clusterset_select_gr_members_status",
          "router_router_select_cs_options",
        ],
        options);


    var common_responses_regex = common_stmts.prepare_statement_responses_regex(
        [
          "router_unknown_clusterset_view_id",
          "router_clusterset_select_cluster_info_by_gr_uuid_unknown",
          "router_clusterset_id",
        ],
        options);

    var router_start_transaction =
        common_stmts.get("router_start_transaction", options);

    var router_update_attributes =
        common_stmts.get("router_update_attributes_v2", options);

    var router_update_last_check_in =
        common_stmts.get("router_update_last_check_in_v2", options);


    var res;
    if (common_responses.hasOwnProperty(stmt)) {
      return common_responses[stmt];
    } else if (stmt === router_start_transaction.stmt) {
      mysqld.global.transaction_count++;
      return router_start_transaction;
    } else if (stmt === router_update_last_check_in.stmt) {
      mysqld.global.update_last_check_in_count++;
      return router_update_last_check_in;
    } else if (
        (res = common_stmts.handle_regex_stmt(stmt, common_responses_regex)) !==
        undefined) {
      return res;
    } else if (stmt.match(router_update_attributes.stmt_regex)) {
      mysqld.global.update_attributes_count++;
      return router_update_attributes;
    } else if (stmt === "set @@mysqlx_wait_timeout = 28800") {
      return {
        ok: {}
      }
    } else if (stmt === "enable_notices") {
      return {
        ok: {}
      }
    } else {
      return common_stmts.unknown_statement_response(stmt);
    }
  }
})
