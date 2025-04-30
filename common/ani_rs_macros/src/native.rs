// Copyright (C) 2025 Huawei Device Co., Ltd.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

use proc_macro::TokenStream;
use proc_macro2::TokenStream as TokenStream2;
use quote::{quote, ToTokens, TokenStreamExt};
use syn::{
    parse::{Parse, ParseStream, Parser},
    punctuated::Punctuated,
    spanned::Spanned,
    token::Comma,
    Attribute, Error, Expr, ItemEnum, ItemFn, ItemStruct, LitStr, MetaNameValue, Result, Stmt,
};

pub(crate) fn entry(args: TokenStream2, item: TokenStream2, output: bool) -> Result<TokenStream2> {
    const ENV: usize = 0;
    const THIS: usize = 1;
    const OBJ: usize = 2;

    let mut item = syn::parse2::<ItemFn>(item)?;
    let item_clone = item.clone();

    let mut block = quote! {};

    let mut sig = quote! {
        env: ani_rs::AniEnv<'local>,
        this: ani_rs::objects::AniObject<'local>,
    };

    let mut input = quote! {};
    for i in item.sig.inputs.iter() {
        if let syn::FnArg::Typed(pat) = i {
            if let syn::Pat::Ident(pat) = &*pat.pat {
                if pat.ident.to_string() == "this" {
                    block = quote! {
                        #block
                        let this = env.deserialize(this).unwrap();
                    };
                    input = quote! {
                        #input
                        this,
                    };
                } else {
                    let pat = pat.ident.clone();
                    block = quote! {
                        #block
                        let #pat = env.deserialize(#pat).unwrap();
                    };
                    input = quote! {
                        #input
                        #pat,
                    };
                    sig = quote! {
                        #sig
                        #pat: ani_rs::objects::AniObject<'local>,
                    }
                }
            }
        }
    }
    let ident = item.sig.ident.clone();
    if output {
        sig = quote! {
        fn #ident<'local>(#sig) -> AniRef<'local>
        };
    } else {
        sig = quote! {fn #ident<'local>(#sig)}
    }

    let mut sig = syn::parse2(sig).unwrap();

    item.sig = sig;

    let block = quote!(

        #item_clone
        #block
        let res = #ident (#input);

    );

    let block = if output {
        quote! {
           { #block
            match res {
                Ok(res) => {
                    env.serialize(&res).unwrap()
                }
                Err(err) => {
                    let res = env.undefined().unwrap();
                    env.throw_business_error(err.code(), err.message())
                        .unwrap();
                    res
                }
            }
            }
        }
    } else {
        quote! {
            {
                #block
                if let Err(err) =  res {
                    env.throw_business_error(err.code(), err.message())
                        .unwrap();
                }
            }
        }
    };

    item.block = syn::parse2(block).unwrap();
    Ok(quote! {
        #item
    })
}
