class Makeme < Formula
  homepage "https://embedthis.com/makeme/"
  url "https://github.com/embedthis/makeme/archive/v0.10.2.tar.gz"
  sha1 "0d8f770d66722aa43e8ef06147da7cba57707e47"

  # head "https://github.com/embedthis/makeme.git"
  # version "0.10.2"

  def install
    ENV.deparallelize
    system "make", "boot"
    system "build/macosx-x64-default/bin/me", "configure", "--release", "--prefix", "app=#{HOMEBREW_PREFIX}/Cellar/#{name}", "compile"
    system "build/macosx-x64-release/bin/me", "install"
  end
  test do
    system "#{bin}/me"
  end
end
