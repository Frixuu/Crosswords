//#[cfg(target_os="android")]
#[allow(non_snake_case)]
pub mod android {

    use jni::JNIEnv;
    use jni::objects::{JClass, JString};
    use jni::sys::{jstring};

    use std::ffi::{CString};

    #[no_mangle]
    pub unsafe extern fn Java_xyz_lukasz_xword_search_MissingLettersIndex_helloWorld(env: JNIEnv, _: JClass) -> jstring {
        // Our Java companion code might pass-in "world" as a string, hence the name.
        //let world = rust_greeting(env.get_string(java_pattern).expect("invalid pattern string").as_ptr());
        // Retake pointer so that we can use it below and allow memory to be freed when it goes out of scope.
        let world_ptr = CString::new("XDDDDDD").expect("cannot create cstring");
        let output = env.new_string(world_ptr.to_str().unwrap()).expect("Couldn't create java string!");
        output.into_raw()
    }
}
