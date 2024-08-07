//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of stateless STUN server to NAPI
//

#include "util/util_result_sugar.hpp"
#include "stun/stun_server_stateless.hpp"
#include "node/openssl/node_openssl_hash.hpp"
#include "node_stun_server_stateless.hpp"
#include "node_stun_message.hpp"
#include <craftnapi/error.hpp>

namespace freewebrtc::node_stun {

using Value = craftnapi::Value;
using Object = craftnapi::Object;
using Env = craftnapi::Env;
using CallbackInfo = craftnapi::CallbackInfo;

namespace {

Result<Value> constructor(Env&, const CallbackInfo& info) {
    return info.this_arg.as_object()
        > [](const auto& obj) { return obj.wrap(std::make_unique<stun::server::Stateless>(crypto::node_openssl::sha1)); };
}

Result<net::Endpoint> extract_rinfo(const Value& v) {
    auto v_as_obj = v.as_object();
    auto addr_rv = (v_as_obj
        > [](auto obj) { return obj.named_property("address"); }
        > [](auto v)   { return v.as_string(); }
        > [](auto v)   { return net::ip::Address::from_string(v); }
        ).add_context("rinfo.address");

    auto port_rv = (v_as_obj
        > [](auto obj) { return obj.named_property("port"); }
        > [](const auto& v) { return v.as_int32(); }
        ).add_context("rinfo.port");

    return combine(
        [](auto&& addr, auto&& port) -> Result<net::Endpoint> {
            return net::Endpoint(net::UdpEndpoint{addr, net::Port(port)});
        },
        std::move(addr_rv),
        std::move(port_rv));
}

Result<Value> process_message(Env& env, const CallbackInfo& info) {
    // (message, rinfo)
    auto buffer_rv = (info[0] > [](const auto& arg) { return arg.as_buffer(); }).add_context("buffer (1st parameter)");
    auto rinfo_rv = (info[1] > [](const auto& arg) { return extract_rinfo(arg); }).add_context("remote info (2nd parameter)");
    auto obj_rvv = info.this_arg.unwrap<stun::server::Stateless>().add_context("object unwrap");

    return combine(
        [&](auto&& message, auto&& endpoint, auto&& server) {
            using RVO = Result<Object>;
            auto result = server.get().process(endpoint, message);
            return std::visit(
                util::overloaded {
                    [&](const stun::server::Stateless::Ignore& ign) -> RVO {
                        return
                            env.create_object({
                                {"result", env.create_string("ignore")},
                                {"message",
                                        std::move(ign.message).fmap([&](const auto& msg) {
                                            return node_stun::message(env, msg);
                                        })}
                            });
                    },
                    [&](const stun::server::Stateless::Error& err) -> RVO {
                        return err.error;
                    },
                    [&](const stun::server::Stateless::Respond& rsp) -> RVO {
                        return
                            env.create_object({
                                {"result", env.create_string("respond")},
                                {"data",   rsp.response.build(rsp.maybe_integrity)
                                        .bind([&](const auto& bytevec) {
                                            return env.create_buffer(util::ConstBinaryView(bytevec));
                                        })}
                            });
                    }
                },
                result).fmap(Object::fmap_to_value);
        }
        , std::move(buffer_rv)
        , std::move(rinfo_rv)
        , std::move(obj_rvv));

}

Result<Value> add_user(Env& env, const CallbackInfo& info) {
    // (username, password)
    auto username_rv = (info[0]
        > [](const auto& arg) { return arg.as_string(); }
        > [](const auto& str) {
            return precis::OpaqueString{str};
        }).add_context("username (1st parameter)");


    auto password_rv = (info[1]
        > [](const auto& arg) { return arg.as_string(); }
        > [](const auto& str) {
            return stun::Password::short_term(precis::OpaqueString{str}, crypto::node_openssl::sha1);
        }).add_context("password (2nd parameter)");

    auto obj_rvv = info.this_arg.unwrap<stun::server::Stateless>().add_context("object unwrap");
    return combine(
        [&](auto&& username, auto&& password, auto&& server) {
            server.get().add_user(username, password);
            return env.create_undefined();
        }
        , std::move(username_rv)
        , std::move(password_rv)
        , std::move(obj_rvv));
}

}

Result<Value> server_stateless_class(Env& env, std::string_view name) {
    return
        env.create_class(
            name,
            constructor,
            {
                {"process", process_message},
                {"addUser", add_user}
            })
        .add_context("stateless stun server class", name);
}

}
