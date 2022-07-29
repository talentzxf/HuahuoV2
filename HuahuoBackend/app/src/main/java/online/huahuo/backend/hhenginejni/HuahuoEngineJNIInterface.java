package online.huahuo.backend.hhenginejni;

class ProjectFileMetaInfo{
    private boolean isValid;
    private String version;

    public boolean isValid() {
        return isValid;
    }

    public void setValid(boolean valid) {
        isValid = valid;
    }

    public String getVersion() {
        return version;
    }

    public void setVersion(String version) {
        this.version = version;
    }
}

class HuahuoEngineJNIInterface{
    static native public ProjectFileMetaInfo getProjectFileMetaInfo(String path);
}