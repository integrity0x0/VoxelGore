plugins {
    alias(libs.plugins.android.application)
}

android {
    namespace = "com.voxelgore"
    compileSdk {
        version = release(37)
    }

    defaultConfig {
        applicationId = "com.voxelgore"
        minSdk = 26
        targetSdk = 37
        versionCode = 18
        versionName = "0.10"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        ndk {
            abiFilters += "arm64-v8a"
        }

    }


    buildTypes {
        release {
            optimization {
                enable = false
            }
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }

    externalNativeBuild {
        cmake {
            path = file("../CMakeLists.txt")
        }
    }

    sourceSets {
        getByName("main") {
            assets.directories.add("../assets")
        }
    }
}

dependencies {
    implementation(libs.appcompat)
    implementation(libs.material)
    testImplementation(libs.junit)
    androidTestImplementation(libs.espresso.core)
    androidTestImplementation(libs.ext.junit)
}